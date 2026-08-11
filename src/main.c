#include "../libft/libft.h"
#include "../include/ft_traceroute.h"

void print_help(void)
{
    printf("Usage: ft_traceroute [--help] <destination>\n");
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

int main(int argc, char **argv)
{
    t_args args;

    args = parse_args(argc, argv);
    printf("Destination: %s\n", args.hostname);
    return (0);
}
