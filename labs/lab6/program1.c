#ifdef ZAD1

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>

static volatile bool running = true;

static frame_t latest_left_frame;
static frame_t latest_right_frame;
static pthread_mutex_t mutex_left = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_right = PTHREAD_MUTEX_INITIALIZER;
static sem_t sem_left_ready;
static sem_t sem_right_ready;

static stereo_pair_t latest_pair;
static pthread_mutex_t mutex_pair = PTHREAD_MUTEX_INITIALIZER;
static sem_t sem_pair_ready;

static robot_state_t latest_robot_state;
static pthread_mutex_t mutex_state = PTHREAD_MUTEX_INITIALIZER;
static sem_t sem_state_ready;

static double f_rand(double min, double max) {
    return min + ((double)rand() / RAND_MAX) * (max - min);
}

void *left_camera_thread_func(void *arg) {
    (void)arg;
    int frame_num = 0;
    struct timespec next;
    get_timestamp(&next);
    while (running) {
        frame_t frame;
        frame.camera_id = 0;
        frame.frame_number = ++frame_num;
        get_timestamp(&frame.timestamp);
        generate_random_image(frame.image_data);

        pthread_mutex_lock(&mutex_left);
        latest_left_frame = frame;
        pthread_mutex_unlock(&mutex_left);

        sem_post(&sem_left_ready);
        sleep_until_next_period(1000000000L / CAMERA_FREQ_HZ, &next);
    }
    return NULL;
}

void *right_camera_thread_func(void *arg) {
    (void)arg;
    int frame_num = 0;
    struct timespec next;
    get_timestamp(&next);
    while (running) {
        frame_t frame;
        frame.camera_id = 1;
        frame.frame_number = ++frame_num;
        get_timestamp(&frame.timestamp);
        generate_random_image(frame.image_data);

        pthread_mutex_lock(&mutex_right);
        latest_right_frame = frame;
        pthread_mutex_unlock(&mutex_right);

        sem_post(&sem_right_ready);
        sleep_until_next_period(1000000000L / CAMERA_FREQ_HZ, &next);
    }
    return NULL;
}

void *sync_thread_func(void *arg) {
    (void)arg;
    int pair_num = 0;
    while (running) {
        sem_wait(&sem_left_ready);
        sem_wait(&sem_right_ready);
        if (!running) break;

        frame_t left;
        pthread_mutex_lock(&mutex_left);
        left = latest_left_frame;
        pthread_mutex_unlock(&mutex_left);

        frame_t right;
        pthread_mutex_lock(&mutex_right);
        right = latest_right_frame;
        pthread_mutex_unlock(&mutex_right);

        double diff = timespec_diff_ms(&left.timestamp, &right.timestamp);
        if (diff < SYNC_THRESHOLD_MS) {
            stereo_pair_t pair;
            pair.left = left;
            pair.right = right;
            pair.pair_number = ++pair_num;

            pthread_mutex_lock(&mutex_pair);
            latest_pair = pair;
            pthread_mutex_unlock(&mutex_pair);

            sem_post(&sem_pair_ready);
        }
    }
    return NULL;
}

void *image_writer_thread_func(void *arg) {
    (void)arg;
    struct timespec next;
    get_timestamp(&next);
    while (running) {
        sem_wait(&sem_pair_ready);
        if (!running) break;

        stereo_pair_t pair;
        pthread_mutex_lock(&mutex_pair);
        pair = latest_pair;
        pthread_mutex_unlock(&mutex_pair);

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
    while (running) {
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

        pthread_mutex_lock(&mutex_state);
        latest_robot_state = state;
        pthread_mutex_unlock(&mutex_state);

        sem_post(&sem_state_ready);
        sleep_until_next_period(1000000000L / ROBOT_STATE_FREQ_HZ, &next);
    }
    return NULL;
}

void *logger_thread_func(void *arg) {
    (void)arg;
    struct timespec next;
    get_timestamp(&next);
    while (running) {
        sem_wait(&sem_state_ready);
        if (!running) break;

        robot_state_t state;
        pthread_mutex_lock(&mutex_state);
        state = latest_robot_state;
        pthread_mutex_unlock(&mutex_state);

        printf("[LOGGER] State seq: %d, Pos: (%.2f, %.2f, %.2f), Orient: (%.2f, %.2f, %.2f)\n",
               state.seq_number, state.pos_x, state.pos_y, state.pos_z,
               state.roll, state.pitch, state.yaw);

        sleep_until_next_period(1000000000L / LOGGER_FREQ_HZ, &next);
    }
    return NULL;
}

int main(void) {
    srand(time(NULL));

    sem_init(&sem_left_ready, 0, 0);
    sem_init(&sem_right_ready, 0, 0);
    sem_init(&sem_pair_ready, 0, 0);
    sem_init(&sem_state_ready, 0, 0);

    pthread_t left_cam, right_cam, sync, writer, state, logger;

    pthread_create(&left_cam, NULL, left_camera_thread_func, NULL);
    pthread_create(&right_cam, NULL, right_camera_thread_func, NULL);
    pthread_create(&sync, NULL, sync_thread_func, NULL);
    pthread_create(&writer, NULL, image_writer_thread_func, NULL);
    pthread_create(&state, NULL, robot_state_thread_func, NULL);
    pthread_create(&logger, NULL, logger_thread_func, NULL);

    sleep(RUN_DURATION_SEC);

    running = false;

    sem_post(&sem_left_ready);
    sem_post(&sem_right_ready);
    sem_post(&sem_pair_ready);
    sem_post(&sem_state_ready);

    pthread_join(left_cam, NULL);
    pthread_join(right_cam, NULL);
    pthread_join(sync, NULL);
    pthread_join(writer, NULL);
    pthread_join(state, NULL);
    pthread_join(logger, NULL);

    sem_destroy(&sem_left_ready);
    sem_destroy(&sem_right_ready);
    sem_destroy(&sem_pair_ready);
    sem_destroy(&sem_state_ready);

    return 0;
}

#endif
