#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <netinet/in.h>

typedef struct s_args {
    char                *hostname;
    struct sockaddr_in  dest;
}   t_args;

t_args  parse_args(int argc, char **argv);
void    print_help(void);

#endif

