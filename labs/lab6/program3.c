#ifdef ZAD3

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <stdatomic.h>
#include <sched.h>

typedef struct {
    stereo_pair_t buffer[BUFFER_CAPACITY];
    int head;
    int tail;
    int count;
    int capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_not_full;
} stereo_pair_buffer_t;

static atomic_bool running = true;

static frame_buffer_t left_cam_buffer;
static frame_buffer_t right_cam_buffer;
static stereo_pair_buffer_t pair_buffer;
static robot_state_buffer_t state_buffer;

static atomic_int left_frames_gen = 0;
static atomic_int right_frames_gen = 0;
static atomic_int pairs_gen = 0;
static atomic_int robot_states_gen = 0;

static void init_pair_buffer(stereo_pair_buffer_t *buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    buf->capacity = BUFFER_CAPACITY;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->cond_not_empty, NULL);
    pthread_cond_init(&buf->cond_not_full, NULL);
}

static void destroy_pair_buffer(stereo_pair_buffer_t *buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->cond_not_empty);
    pthread_cond_destroy(&buf->cond_not_full);
}

static void push_pair(stereo_pair_buffer_t *buf, const stereo_pair_t *pair, atomic_bool *run_flag) {
    pthread_mutex_lock(&buf->mutex);
    if (atomic_load(run_flag)) {
        if (buf->count == buf->capacity) {
            buf->head = (buf->head + 1) % buf->capacity;
            buf->count--;
        }
        buf->buffer[buf->tail] = *pair;
        buf->tail = (buf->tail + 1) % buf->capacity;
        buf->count++;
        pthread_cond_signal(&buf->cond_not_empty);
    }
    pthread_mutex_unlock(&buf->mutex);
}

static bool pop_pair(stereo_pair_buffer_t *buf, stereo_pair_t *pair, atomic_bool *run_flag) {
    bool success = false;
    pthread_mutex_lock(&buf->mutex);
    while (buf->count == 0 && atomic_load(run_flag)) {
        pthread_cond_wait(&buf->cond_not_empty, &buf->mutex);
    }
    if (atomic_load(run_flag) && buf->count > 0) {
        *pair = buf->buffer[buf->head];
        buf->head = (buf->head + 1) % buf->capacity;
        buf->count--;
        success = true;
        pthread_cond_signal(&buf->cond_not_full);
    }
    pthread_mutex_unlock(&buf->mutex);
    return success;
}

static void broadcast_all_buffers(void) {
    pthread_mutex_lock(&left_cam_buffer.mutex);
    pthread_cond_broadcast(&left_cam_buffer.cond_not_empty);
    pthread_cond_broadcast(&left_cam_buffer.cond_not_full);
    pthread_mutex_unlock(&left_cam_buffer.mutex);

    pthread_mutex_lock(&right_cam_buffer.mutex);
    pthread_cond_broadcast(&right_cam_buffer.cond_not_empty);
    pthread_cond_broadcast(&right_cam_buffer.cond_not_full);
    pthread_mutex_unlock(&right_cam_buffer.mutex);

    pthread_mutex_lock(&pair_buffer.mutex);
    pthread_cond_broadcast(&pair_buffer.cond_not_empty);
    pthread_cond_broadcast(&pair_buffer.cond_not_full);
    pthread_mutex_unlock(&pair_buffer.mutex);

    pthread_mutex_lock(&state_buffer.mutex);
    pthread_cond_broadcast(&state_buffer.cond_not_empty);
    pthread_cond_broadcast(&state_buffer.cond_not_full);
    pthread_mutex_unlock(&state_buffer.mutex);
}

static void handle_sigint(int sig) {
    (void)sig;
    atomic_store(&running, false);
    broadcast_all_buffers();
}

static double f_rand(double min, double max) {
    return min + ((double)rand() / RAND_MAX) * (max - min);
}

void *left_camera_thread_func(void *arg) {
    (void)arg;
    int frame_num = 0;
    struct timespec next;
    get_timestamp(&next);
    while (atomic_load(&running)) {
        frame_t frame;
        frame.camera_id = 0;
        frame.frame_number = ++frame_num;
        get_timestamp(&frame.timestamp);
        generate_random_image(frame.image_data);

        push_frame(&left_cam_buffer, &frame, (bool*)&running);
        atomic_fetch_add(&left_frames_gen, 1);

        sleep_until_next_period(1000000000L / CAMERA_FREQ_HZ, &next);
    }
    return NULL;
}

void *right_camera_thread_func(void *arg) {
    (void)arg;
    int frame_num = 0;
    struct timespec next;
    get_timestamp(&next);
    while (atomic_load(&running)) {
        frame_t frame;
        frame.camera_id = 1;
        frame.frame_number = ++frame_num;
        get_timestamp(&frame.timestamp);
        generate_random_image(frame.image_data);

        push_frame(&right_cam_buffer, &frame, (bool*)&running);
        atomic_fetch_add(&right_frames_gen, 1);

        sleep_until_next_period(1000000000L / CAMERA_FREQ_HZ, &next);
    }
    return NULL;
}

void *sync_thread_func(void *arg) {
    (void)arg;
    int pair_num = 0;
    bool has_left = false;
    bool has_right = false;
    frame_t left_frame;
    frame_t right_frame;

    while (atomic_load(&running)) {
        if (!has_left) {
            if (!pop_frame(&left_cam_buffer, &left_frame, (bool*)&running)) {
                break;
            }
            has_left = true;
        }
        if (!has_right) {
            if (!pop_frame(&right_cam_buffer, &right_frame, (bool*)&running)) {
                break;
            }
            has_right = true;
        }

        double diff = timespec_diff_ms(&left_frame.timestamp, &right_frame.timestamp);
        if (diff < SYNC_THRESHOLD_MS) {
            stereo_pair_t pair;
            pair.left = left_frame;
            pair.right = right_frame;
            pair.pair_number = ++pair_num;

            push_pair(&pair_buffer, &pair, &running);
            atomic_fetch_add(&pairs_gen, 1);

            has_left = false;
            has_right = false;
        } else {
            double t_left = left_frame.timestamp.tv_sec + left_frame.timestamp.tv_nsec / 1e9;
            double t_right = right_frame.timestamp.tv_sec + right_frame.timestamp.tv_nsec / 1e9;
            if (t_left < t_right) {
                has_left = false;
            } else {
                has_right = false;
            }
        }
    }
    return NULL;
}

void *image_writer_thread_func(void *arg) {
    (void)arg;
    struct timespec next;
    get_timestamp(&next);
    while (atomic_load(&running)) {
        stereo_pair_t pair;
        if (!pop_pair(&pair_buffer, &pair, &running)) {
            break;
        }

        printf("[WRITER] left_%04d.jpg: %s\n", pair.pair_number, pair.left.image_data);
        printf("[WRITER] right_%04d.jpg: %s\n", pair.pair_number, pair.right.image_data);

        sleep_until_next_period(1000000000L / IMAGE_WRITE_FREQ_HZ, &next);
    }
    return NULL;
}

void *robot_state_thread_func(void *arg) {
    (void)arg;
    int seq = 0;
    struct timespec next;
    get_timestamp(&next);
    double px = 0.0, py = 0.0, pz = 0.0;
    double roll = 0.0, pitch = 0.0, yaw = 0.0;
    while (atomic_load(&running)) {
        robot_state_t state;
        state.seq_number = ++seq;
        get_timestamp(&state.timestamp);
        px += f_rand(-0.1, 0.1);
        py += f_rand(-0.1, 0.1);
        pz += f_rand(-0.01, 0.01);
        roll += f_rand(-0.5, 0.5);
        pitch += f_rand(-0.5, 0.5);
        yaw += f_rand(-1.0, 1.0);
        state.pos_x = px;
        state.pos_y = py;
        state.pos_z = pz;
        state.roll = roll;
        state.pitch = pitch;
        state.yaw = yaw;

        push_robot_state(&state_buffer, &state, (bool*)&running);
        atomic_fetch_add(&robot_states_gen, 1);

        sleep_until_next_period(1000000000L / ROBOT_STATE_FREQ_HZ, &next);
    }
    return NULL;
}

void *logger_thread_func(void *arg) {
    (void)arg;
    struct timespec next;
    get_timestamp(&next);
    while (atomic_load(&running)) {
        robot_state_t state;
        if (!pop_robot_state(&state_buffer, &state, (bool*)&running)) {
            break;
        }

        printf("[LOGGER] State seq: %d, Pos: (%.2f, %.2f, %.2f), Orient: (%.2f, %.2f, %.2f)\n",
               state.seq_number, state.pos_x, state.pos_y, state.pos_z,
               state.roll, state.pitch, state.yaw);

        sleep_until_next_period(1000000000L / LOGGER_FREQ_HZ, &next);
    }
    return NULL;
}

static void *watchdog_thread_func(void *arg) {
    (void)arg;
    int last_l = 0;
    int last_r = 0;
    int last_s = 0;
    struct timespec last_time;
    get_timestamp(&last_time);

    while (atomic_load(&running)) {
        sleep(1);
        if (!atomic_load(&running)) break;

        struct timespec curr_time;
        get_timestamp(&curr_time);
        double elapsed = timespec_diff_ms(&curr_time, &last_time) / 1000.0;
        if (elapsed <= 0.0) continue;

        int curr_l = atomic_load(&left_frames_gen);
        int curr_r = atomic_load(&right_frames_gen);
        int curr_s = atomic_load(&robot_states_gen);

        double freq_l = (curr_l - last_l) / elapsed;
        double freq_r = (curr_r - last_r) / elapsed;
        double freq_s = (curr_s - last_s) / elapsed;

        if (freq_l < 20.0) {
            printf("[WATCHDOG] LEFT CAMERA SLOW (%.2f Hz)\n", freq_l);
        }
        if (freq_r < 20.0) {
            printf("[WATCHDOG] RIGHT CAMERA SLOW (%.2f Hz)\n", freq_r);
        }
        if (freq_s < 80.0) {
            printf("[WATCHDOG] ROBOT STATE SLOW (%.2f Hz)\n", freq_s);
        }

        last_l = curr_l;
        last_r = curr_r;
        last_s = curr_s;
        last_time = curr_time;
    }
    return NULL;
}

static void set_thread_priority(pthread_t thread, int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    int ret = pthread_setschedparam(thread, SCHED_FIFO, &param);
    if (ret != 0) {
        printf("[RT] Warning: Failed to set thread priority to %d (requires root/CAP_SYS_NICE): %d\n", priority, ret);
    }
}

int main(void) {
    srand(time(NULL));

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    init_frame_buffer(&left_cam_buffer);
    init_frame_buffer(&right_cam_buffer);
    init_pair_buffer(&pair_buffer);
    init_robot_state_buffer(&state_buffer);

    pthread_t left_cam, right_cam, sync, writer, state, logger, watchdog;

    pthread_create(&left_cam, NULL, left_camera_thread_func, NULL);
    pthread_create(&right_cam, NULL, right_camera_thread_func, NULL);
    pthread_create(&sync, NULL, sync_thread_func, NULL);
    pthread_create(&writer, NULL, image_writer_thread_func, NULL);
    pthread_create(&state, NULL, robot_state_thread_func, NULL);
    pthread_create(&logger, NULL, logger_thread_func, NULL);
    pthread_create(&watchdog, NULL, watchdog_thread_func, NULL);

    set_thread_priority(state, 50);
    set_thread_priority(left_cam, 40);
    set_thread_priority(right_cam, 40);
    set_thread_priority(watchdog, 35);
    set_thread_priority(sync, 25);
    set_thread_priority(writer, 20);
    set_thread_priority(logger, 20);

    struct timespec start_time, end_time;
    get_timestamp(&start_time);

    int elapsed = 0;
    while (atomic_load(&running) && elapsed < RUN_DURATION_SEC) {
        sleep(5);
        elapsed += 5;
        if (atomic_load(&running)) {
            int l = atomic_load(&left_frames_gen);
            int r = atomic_load(&right_frames_gen);
            int p = atomic_load(&pairs_gen);
            int s = atomic_load(&robot_states_gen);
            printf("--- SYSTEM STATS (%ds) ---\n", elapsed);
            printf("Left Cam Frames: %d (%.2f Hz)\n", l, (double)l / elapsed);
            printf("Right Cam Frames: %d (%.2f Hz)\n", r, (double)r / elapsed);
            printf("Stereo Pairs: %d\n", p);
            printf("Robot States: %d (%.2f Hz)\n", s, (double)s / elapsed);
            printf("---------------------------\n");
        }
    }

    atomic_store(&running, false);
    broadcast_all_buffers();

    pthread_join(left_cam, NULL);
    pthread_join(right_cam, NULL);
    pthread_join(sync, NULL);
    pthread_join(writer, NULL);
    pthread_join(state, NULL);
    pthread_join(logger, NULL);
    pthread_join(watchdog, NULL);

    get_timestamp(&end_time);
    double total_runtime = timespec_diff_ms(&end_time, &start_time) / 1000.0;

    destroy_frame_buffer(&left_cam_buffer);
    destroy_frame_buffer(&right_cam_buffer);
    destroy_pair_buffer(&pair_buffer);
    destroy_robot_state_buffer(&state_buffer);

    int l = atomic_load(&left_frames_gen);
    int r = atomic_load(&right_frames_gen);
    int p = atomic_load(&pairs_gen);
    int s = atomic_load(&robot_states_gen);

    printf("=== report.txt ===\n");
    printf("Total Left Cam Frames: %d\n", l);
    printf("Total Right Cam Frames: %d\n", r);
    printf("Total Stereo Pairs: %d\n", p);
    printf("Total Robot States: %d\n", s);
    printf("Runtime: %.3fs\n", total_runtime);
    printf("==================\n");

    FILE *f_report = fopen("report.txt", "w");
    if (f_report) {
        fprintf(f_report, "Total Left Cam Frames: %d\n", l);
        fprintf(f_report, "Total Right Cam Frames: %d\n", r);
        fprintf(f_report, "Total Stereo Pairs: %d\n", p);
        fprintf(f_report, "Total Robot States: %d\n", s);
        fprintf(f_report, "Runtime: %.3fs\n", total_runtime);
        fclose(f_report);
    }

    return 0;
}

#endif
