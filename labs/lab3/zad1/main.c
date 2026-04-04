#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

long double fun(long double x) {
    return 4/(x*x + 1);
}

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        return -1;
    }
    long long k = atol(argv[1]);
    int n = atoi(argv[2]);

    long double a = 0.0L, b = 1.0L;
    
    printf("a = %Lf, b = %Lf\n", a, b);

    for (int p = 1; p <= n; ++p) {
        struct timespec time_start, time_stop;
        clock_gettime(CLOCK_MONOTONIC, &time_start);

        long long chunk = k / p;
        long double step = (b - a) / k;

        int fd[p][2];

        for (int i = 0; i < p; ++i) {
            if (pipe(fd[i]) == -1) {
                perror("pipe");
                exit(1);
            }

            long long start = i * chunk;
            long long end = (i == p - 1) ? k : (i + 1) * chunk;

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                close(fd[i][0]);
                long double total = 0;
                long double x = a + start * step;
                for(long long j = start; j < end; ++j){
                    total += step * fun(x);
                    x += step;
                }
                if (write(fd[i][1], &total, sizeof(long double)) < sizeof(long double)) {
                    perror("write");
                    exit(1);
                }
                close(fd[i][1]);
                exit(0);
            }
            close(fd[i][1]);
        }

        while (wait(NULL) > 0) { };

        long double answer = 0;
        for (int i = 0; i < p; ++i) {
            long double total;
            if (read(fd[i][0], &total, sizeof(long double)) < sizeof(long double)) {
                perror("read");
                exit(1);
            }
            close(fd[i][0]);
            answer += total;
        }

        clock_gettime(CLOCK_MONOTONIC, &time_stop);
        double time_spent = (time_stop.tv_sec - time_start.tv_sec) + (time_stop.tv_nsec - time_start.tv_nsec) / 1e9;
        printf("process count = %d, answer = %.15Lf, time = %lf\n", p, answer, time_spent);
        fflush(stdout);
    }
}
