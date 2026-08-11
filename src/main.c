# include "../include/ft_traceroute.h"
# include <sys/socket.h>
# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h> 
# include <netinet/ip.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <errno.h>
# include <arpa/inet.h>

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

void set_socket_timeout(int sock, int seconds)
{
    struct timeval tv;

    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void send_probe(int sock, struct sockaddr_in *dest, int ttl)
{
    struct icmphdr icmp;

    setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    
    memset(&icmp, 0, sizeof(icmp));
    icmp.type = ICMP_ECHO;
    icmp.un.echo.id = getpid() & 0xFFFF;
    icmp.un.echo.sequence = ttl;
    icmp.checksum = 0;
    icmp.checksum = checksum(&icmp, sizeof(icmp));

    sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)dest, sizeof(*dest));
}

int recv_probe(int sock, struct sockaddr_in *reply_addr)
{
    char buf[512];
    struct iphdr *ip;
    struct icmphdr *icmp;
    struct iphdr *inner_ip;
    struct icmphdr *inner_icmp;
    socklen_t len = sizeof(*reply_addr);
    ssize_t ret;

    ret = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)reply_addr, &len);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (-2);
        return (-1);
    }
    ip = (struct iphdr *)buf;
    icmp = (struct icmphdr *)(buf + ip->ihl * 4);

    if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_DEST_UNREACH)
    {
        inner_ip = (struct iphdr *)((char *)icmp + 8);
        inner_icmp = (struct icmphdr *)((char *)inner_ip + inner_ip->ihl * 4);
        if (inner_icmp->un.echo.id != (getpid() & 0xFFFF))
            return (-1);
        return (icmp->type);
    }
    if (icmp->un.echo.id != (getpid() & 0xFFFF))
        return (-1);
    return (icmp->type);
}

int main(int argc, char **argv)
{
    t_args args;
    struct sockaddr_in reply_addr;
    int sock;
    int type;

    args = parse_args(argc, argv);
    resolve_hostname(&args);
    printf("ft_traceroute to %s (%s), %d hops max\n", args.hostname, inet_ntoa(args.dest.sin_addr), MAX_HOPS);
    sock = create_socket();
    set_socket_timeout(sock, 3);

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++)
    {
        int printed_ip = 0;
        printf("%2d ", ttl);

        char *hop_d = NULL;
        for (int probe = 0; probe < 3; probe++)
        {
            struct timeval start, end;
            double elapsed;

            gettimeofday(&start, NULL);
            send_probe(sock, &args.dest, ttl);
            type = recv_probe(sock, &reply_addr);
            while (type == -1)
            {
                gettimeofday(&end, NULL);
                elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                if (elapsed > 3000.0)
                {
                    type = -2;
                    break;
                }
                type = recv_probe(sock, &reply_addr);
            }
            gettimeofday(&end, NULL);
            if (!printed_ip && type >= 0)
            {
                printf("%s ", inet_ntoa(reply_addr.sin_addr));
                printed_ip = 1;
            }
            if (type == -2)
                printf("* ");
            else
            {
                double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                printf("%.3f ms ", rtt);
            }
        }
        printf("\n");
        if (reply_addr.sin_addr.s_addr == args.dest.sin_addr.s_addr)
            break;
    }

    close(sock);
    return (0);
}
