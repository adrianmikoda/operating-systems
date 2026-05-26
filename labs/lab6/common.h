#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <pthread.h>

#if defined(ZAD2) || defined(ZAD3)
#include <stdbool.h>
#endif

#ifdef ZAD3
#include <stdatomic.h>
#endif

#define CAMERA_FREQ_HZ 25
#define ROBOT_STATE_FREQ_HZ 100
#define IMAGE_WRITE_FREQ_HZ 10
#define LOGGER_FREQ_HZ 10
#define SYNC_THRESHOLD_MS 20
#define RUN_DURATION_SEC 20
#define IMAGE_DATA_LEN 10
#define BUFFER_CAPACITY 64

typedef struct {
    int camera_id;
    int frame_number;
    struct timespec timestamp;
    char image_data[IMAGE_DATA_LEN + 1];
} frame_t;

typedef struct {
    frame_t left;
    frame_t right;
    int pair_number;
} stereo_pair_t;

typedef struct {
    double pos_x;
    double pos_y;
    double pos_z;
    double roll;
    double pitch;
    double yaw;
    struct timespec timestamp;
    int seq_number;
} robot_state_t;

#if defined(ZAD2) || defined(ZAD3)
typedef struct {
    frame_t buffer[BUFFER_CAPACITY];
    int head;
    int tail;
    int count;
    int capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_not_full;
} frame_buffer_t;

typedef struct {
    robot_state_t buffer[BUFFER_CAPACITY];
    int head;
    int tail;
    int count;
    int capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_not_full;
} robot_state_buffer_t;
#endif

void generate_random_image(char *buf);
void get_timestamp(struct timespec *ts);
double timespec_diff_ms(struct timespec *t1, struct timespec *t2);
void sleep_until_next_period(long period_ns, struct timespec *next);

#if defined(ZAD2) || defined(ZAD3)
void init_frame_buffer(frame_buffer_t *buf);
void destroy_frame_buffer(frame_buffer_t *buf);
void push_frame(frame_buffer_t *buf, const frame_t *frame, bool *running);
bool pop_frame(frame_buffer_t *buf, frame_t *frame, bool *running);

void init_robot_state_buffer(robot_state_buffer_t *buf);
void destroy_robot_state_buffer(robot_state_buffer_t *buf);
void push_robot_state(robot_state_buffer_t *buf, const robot_state_t *state, bool *running);
bool pop_robot_state(robot_state_buffer_t *buf, robot_state_t *state, bool *running);
#endif

#endif
