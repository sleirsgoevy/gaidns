# gaidns

This is a `getaddrinfo`-based DNS server. Useful if you have an uncommon way to resolve DNS and want to turn that into a proper nameserver.

The server itself parses incoming DNS requests, queries `getaddrinfo` for the IP addresses, and crafts a valid DNS response with those addresses. This means that it can only resolve A and AAAA records, not TXT or anything else.

## Building

Just type `make` to build `gaidns`

## Running

`gaidns` binds to port 53/udp on all available network interfaces. TCP is not supported, as it's not necessary for name resolving to work.

## Customization

The server consists of two files: the protocol implementation in `main.c`, and the business logic in `handlers.c`. The API used by `handlers.c` is as follows:

`void handle(char* domain, int record_type, void* opaque);`

This function is called on each incoming request. `domain` is a dot-separated domain name without a trailing dot. `record_type` is the DNS record type of the request.

`int write_record(void* opaque, int domain, int record_type, void* data, int len);`

This function can be used to add a record into the response for the current request.

`opaque` must be the same that has been passed to `handle`.

`domain` refers to the domain name the record belongs to. Must be 0 for the actually-requested domain.

`record_type` is the DNS record type of the record.

`data` is the actual data carried in the record. For A and AAAA records that is the address itself.

`len` is the length of `data`.

If `data` is another hostname, then the return value of `write_record` can be passed as the `domain` argument for subsequent calls.
