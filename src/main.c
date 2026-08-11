#include "../libft/libft.h"
#include "../include/ft_traceroute.h"
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void print_help(void)
{
    printf("Usage: ft_traceroute [--help] <destination>\n");
}

unsigned short checksum(void *buf, int len)
{
    unsigned short *data = buf;
    unsigned int sum = 0;

    while (len > 1)
    {
        sum += *data++;
        len -= 2;
    }
    if (len == 1)
        sum += *(unsigned char *)data;
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >>16);
    return (~sum);
}

t_args parse_args(int argc, char **argv)
{
    t_args args;

    memset(&args, 0, sizeof(t_args));
    if (argc < 2)
    {
        printf("ft_traceroute: missing destination\n");
        print_help();
        exit(1);
    }
    if (strcmp(argv[1], "--help") == 0)
    {
        print_help();
        exit(0);
    }
    args.hostname = argv[1];
    return (args);
}

void resolve_hostname(t_args *args)
{
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    int addrerror = getaddrinfo(args->hostname, NULL, &hints, &res);
    if (addrerror)
    {
        fprintf(stderr, "ft_traceroute: %s\n", gai_strerror(addrerror));
        exit(1);
    }
    memcpy(&args->dest, res->ai_addr, sizeof(args->dest));
    freeaddrinfo(res);
}
int create_socket(void)
{
    int sock;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        perror("ft_traceroute: socket");
        exit(1);
    }
    return (sock);
}

void send_probe(int sock, struct sockaddr_in *dest, int ttl)
{
    struct icmphdr icmp;

    setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    
    memset(&icmp, 0, sizeof(icmp));
    icmp.type = ICMP_ECHO;
    icmp.un.echo.id = getpid();
    icmp.un.echo.sequence = ttl;
    icmp.checksum = 0;
    icmp.checksum = checksum(&icmp, sizeof(icmp));

    sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)dest, sizeof(*dest));
}

int main(int argc, char **argv)
{
    t_args args;

    args = parse_args(argc, argv);

    resolve_hostname(&args);
    int sock = create_socket();
    for (int ttl = 1; ttl <= MAX_HOPS; ttl++)
    {
        send_probe(sock, &args.dest, ttl);
        printf("%2d \n", ttl);
    }

    close(sock);
    return (0);
}
