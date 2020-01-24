#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int write_record(void* opaque, int ptr, int type, void* data, int l);

void handle_a(char* host, void* opaque)
{
    struct addrinfo* res;
    if(getaddrinfo(host, "80", NULL, &res))
        return;
    while(res)
    {
        if(res->ai_family == AF_INET && res->ai_socktype == SOCK_STREAM)
            write_record(opaque, 0, 1, &((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr, 4);
        res = res->ai_next;
    }
    freeaddrinfo(res);
}

void handle_aaaa(char* host, void* opaque)
{
    struct addrinfo* res;
    if(getaddrinfo(host, "80", NULL, &res))
        return;
    while(res)
    {
        if(res->ai_family == AF_INET6 && res->ai_socktype == SOCK_STREAM)
            write_record(opaque, 0, 0x1c, ((struct sockaddr_in6*)res->ai_addr)->sin6_addr.s6_addr, 16);
        res = res->ai_next;
    }
    freeaddrinfo(res);
}

void handle(char* host, int record_type, void* opaque)
{
    if(record_type == 1)
        handle_a(host, opaque);
    else if(record_type == 0x1c)
        handle_aaaa(host, opaque);
}
