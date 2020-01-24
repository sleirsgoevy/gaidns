#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define perror_if(a, b) do{if(a){perror(b);exit(1);}}while(0)

void handle(char* name, int rtype, void* opaque);

int write_record(void* opaque, int ptr, int type, void* data, int l)
{
    char* ptr0 = ((void**)opaque)[0];
    char* wptr = ((void**)opaque)[1];
    ptr = 0xc000|(ptr+12);
    *wptr++ = ptr >> 8;
    *wptr++ = ptr;
    *wptr++ = type >> 8;
    *wptr++ = type;
    memcpy(wptr, "\0\1\0\0\0;", 6);
    wptr += 6;
    *wptr++ = l >> 8;
    *wptr++ = l;
    int ans = wptr - ptr0;
    memcpy(wptr, data, l);
    wptr += l;
    int nq = ((ptr0[6] & 255) << 8) | (ptr0[7] & 255);
    nq++;
    ptr0[6] = nq >> 8;
    ptr0[7] = nq;
    ((void**)opaque)[1] = wptr;
    return ans - 12;
}

int main()
{
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    perror_if(sock < 0, "socket");
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    memset(addr.sin6_addr.s6_addr, 0, 16);
    addr.sin6_port = ntohs(53);
    perror_if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)), "bind");
    char buf[1048576];
    for(;;)
    {
        socklen_t addrlen = sizeof(addr);
        ssize_t sz = recvfrom(sock, buf, 1048575, 0, (struct sockaddr*)&addr, &addrlen);
        perror_if(sz <= 0 || addrlen != sizeof(addr), "recvfrom");
        buf[1048575] = 0;
        int l = strlen(buf + 12);
        if(l >= sz - 15)
        {
            fprintf(stderr, "invalid DNS packet received");
            continue;
        }
        int idx = 12;
        while(buf[idx])
        {
            int c = buf[idx];
            buf[idx] = '.';
            idx += c + 1;
        }
        int record_type = ((buf[idx + 1] & 255) << 8) | (buf[idx + 2] & 255);
        memcpy(buf+2, "\x81\x80\0\1\0\0\0\0\0\1", 10);
        buf[idx+3] = 0;
        buf[idx+4] = 1;
        char* ptr[2] = {buf, buf+idx+5};
        handle(buf+13, record_type, ptr);
        char* p = buf + 12;
        char* q;
        while((q = strchr(p+1, '.')))
        {
            p[0] = q - p - 1;
            p = q;
        }
        p[0] = strlen(p+1);
        memcpy(ptr[1], "\0\0)\2\0\0\0\0\0\0\0", 11);
        l = ptr[1]-ptr[0]+11;
        perror_if(sendto(sock, buf, l, 0, (struct sockaddr*)&addr, sizeof(addr)) != l, "sendto");
    }
    return 0;
}
