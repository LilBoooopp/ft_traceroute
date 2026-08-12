#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h> 
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>

# define MAX_HOPS 30

typedef struct s_args {
    char                *hostname;
    struct sockaddr_in  dest;
    int max_hops;
}   t_args;

t_args  parse_args(int argc, char **argv);
void    print_help(void);

void resolve_hostname(t_args *args);
int create_socket(void);

#endif

