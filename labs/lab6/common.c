#include "common.h"
#include <stdlib.h>
#include <string.h>

void generate_random_image(char *buf) {
    for (int i = 0; i < IMAGE_DATA_LEN; i++) {
        buf[i] = 'a' + (rand() % 26);
    }
    buf[IMAGE_DATA_LEN] = '\0';
}

void get_timestamp(struct timespec *ts) {
    clock_gettime(CLOCK_MONOTONIC, ts);
}

double timespec_diff_ms(struct timespec *t1, struct timespec *t2) {
    double s = (double)(t1->tv_sec - t2->tv_sec);
    double ns = (double)(t1->tv_nsec - t2->tv_nsec);
    double diff = s * 1000.0 + ns / 1000000.0;
    return diff < 0 ? -diff : diff;
}

void sleep_until_next_period(long period_ns, struct timespec *next) {
    next->tv_nsec += period_ns;
    if (next->tv_nsec >= 1000000000L) {
        next->tv_sec += next->tv_nsec / 1000000000L;
        next->tv_nsec %= 1000000000L;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, next, NULL);
}

#if defined(ZAD2) || defined(ZAD3)
void init_frame_buffer(frame_buffer_t *buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    buf->capacity = BUFFER_CAPACITY;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->cond_not_empty, NULL);
    pthread_cond_init(&buf->cond_not_full, NULL);
}

void destroy_frame_buffer(frame_buffer_t *buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->cond_not_empty);
    pthread_cond_destroy(&buf->cond_not_full);
}

void push_frame(frame_buffer_t *buf, const frame_t *frame, bool *running) {
    pthread_mutex_lock(&buf->mutex);
    if (*running) {
        if (buf->count == buf->capacity) {
            buf->head = (buf->head + 1) % buf->capacity;
            buf->count--;
        }
        buf->buffer[buf->tail] = *frame;
        buf->tail = (buf->tail + 1) % buf->capacity;
        buf->count++;
        pthread_cond_signal(&buf->cond_not_empty);
    }
    pthread_mutex_unlock(&buf->mutex);
}

bool pop_frame(frame_buffer_t *buf, frame_t *frame, bool *running) {
    bool success = false;
    pthread_mutex_lock(&buf->mutex);
    while (buf->count == 0 && *running) {
        pthread_cond_wait(&buf->cond_not_empty, &buf->mutex);
    }
    if (*running && buf->count > 0) {
        *frame = buf->buffer[buf->head];
        buf->head = (buf->head + 1) % buf->capacity;
        buf->count--;
        success = true;
        pthread_cond_signal(&buf->cond_not_full);
    }
    pthread_mutex_unlock(&buf->mutex);
    return success;
}

void init_robot_state_buffer(robot_state_buffer_t *buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    buf->capacity = BUFFER_CAPACITY;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->cond_not_empty, NULL);
    pthread_cond_init(&buf->cond_not_full, NULL);
}

void destroy_robot_state_buffer(robot_state_buffer_t *buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->cond_not_empty);
    pthread_cond_destroy(&buf->cond_not_full);
}

void push_robot_state(robot_state_buffer_t *buf, const robot_state_t *state, bool *running) {
    pthread_mutex_lock(&buf->mutex);
    if (*running) {
        if (buf->count == buf->capacity) {
            buf->head = (buf->head + 1) % buf->capacity;
            buf->count--;
        }
        buf->buffer[buf->tail] = *state;
        buf->tail = (buf->tail + 1) % buf->capacity;
        buf->count++;
        pthread_cond_signal(&buf->cond_not_empty);
    }
    pthread_mutex_unlock(&buf->mutex);
}

bool pop_robot_state(robot_state_buffer_t *buf, robot_state_t *state, bool *running) {
    bool success = false;
    pthread_mutex_lock(&buf->mutex);
    while (buf->count == 0 && *running) {
        pthread_cond_wait(&buf->cond_not_empty, &buf->mutex);
    }
    if (*running && buf->count > 0) {
        *state = buf->buffer[buf->head];
        buf->head = (buf->head + 1) % buf->capacity;
        buf->count--;
        success = true;
        pthread_cond_signal(&buf->cond_not_full);
    }
    pthread_mutex_unlock(&buf->mutex);
    return success;
}
#endif
