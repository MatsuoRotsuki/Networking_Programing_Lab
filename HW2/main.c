#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

int isIPAddress(const char *input) {
    struct sockaddr_in sa;
    return (inet_pton(AF_INET, input, &(sa.sin_addr)) == 1);
}

void resolveDomainToIP(const char *domain) {
    struct addrinfo hints, *result, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(domain, NULL, &hints, &result);

    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        printf("Not found information\n");
        return;
    }

    //printf("IP addresses for domain %s:\n", domain);
    for (res = result; res != NULL; res = res->ai_next) {
        void *addr;
        char ipstr[INET6_ADDRSTRLEN];
        if (res->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            addr = &(ipv4->sin_addr);
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
        inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
        if (res == result) {
            printf("Official IP: ");
        }
        printf("%s\n", ipstr);
    }

    freeaddrinfo(result);
}

void resolveIPToDomain(const char *ip) {
    struct addrinfo hints, *result, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(ip, NULL, &hints, &result);

    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        printf("Not found information\n");
        return;
    }

    //printf("Domain names for IP address %s:\n", ip);
    for (res = result; res != NULL; res = res->ai_next) {
        char host[NI_MAXHOST];
        ret = getnameinfo(res->ai_addr, res->ai_addrlen, host, sizeof(host), NULL, 0, 0);
        if (ret != 0) {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(ret));
            continue;
        }
        if (res == result) {
            printf("Official name: ");
        }
        printf("%s\n", host);
    }

    freeaddrinfo(result);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <domain or IP>\n", argv[0]);
        return 1;
    }

    const char *input = argv[1];
    if (isIPAddress(input)) {
        resolveIPToDomain(input);
    } else {
        resolveDomainToIP(input);
    }

    return 0;
}
