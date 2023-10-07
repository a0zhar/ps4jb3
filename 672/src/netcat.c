#include <ps4/mmap.h>
#include <librop/pthread_create.h>
#include <librop/extcall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>

/*Return values meanings (main_ret):
    2 - socket() failed
    3 - bind() failed
    4 - listen() failed
    5 - accept() failed
    6 - mmap() failed
    9 - success
    1 - setuid(0) work

    PLACE THIS AT THE BOTTOM OF NETCAT.js:
    switch (main_ret) {
        case 2:alert("socket() failed");break;
        case 3:alert("bind() failed");break;
        case 4:alert("listen() failed");break;
        case 5:alert("accept() failed");break;
        case 6:alert("mmap() failed");break;
        case 9:alert("NETCAT SUCCESS!!!!");break;
        case 1:alert("setuid(0) worked");break;
        default:alert("Unknown return value: " + main_ret);break;
    }
*/
#define BLOBMAXSIZE 131072

// We currently only can copy data byte by byte but im working on, implementing 
// a feature (for larger lengths) where it copyies data in chunks
// Replacement for memcpy
void _memcpy_(const void* _Src, void* _Dst, size_t _Len) {
    typedef unsigned char* _UCHAR;
    const _UCHAR srcBuf = (const _UCHAR)_Src;
    _UCHAR dstBuf = (_UCHAR)_Dst;

    // Copy the contents of the source buffer into the 
    // destination byte by byte.
    for (size_t i = 0; i < _Len; i++)
        dstBuf[i] = srcBuf[i];
}

void* sender_thread(void* _) {
    char* mira_blob_2 = __builtin_gadget_addr("$(window.mira_blob_2||0)");
    int mira_blob_2_len = __builtin_gadget_addr("$(window.mira_blob_2_len||0)");
    if (!mira_blob_2) return NULL;
    nanosleep("\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", NULL);
    int q = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0x100007f; // inet_aton(127.0.0.1)
    addr.sin_port = 0x3d23; // htons(9021)

    connect(q, &addr, sizeof(addr));
    char* x = mira_blob_2;
    int l = mira_blob_2_len;
    while (l) {
        int chk = write(q, x, l);
        if (chk <= 0)
            break;
        x += chk;
        l -= chk;
    }
    close(q);
    return NULL;
}

int main() {
    if (setuid(0)) return 1;
    // Create a memory mapping with (read, write, and execute) permissions.
    char* mapping = mmap(NULL, BLOBMAXSIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED)
        return 6;
    // Retrieve the payload data from the JS variable.
    char* mira_blob = __builtin_gadget_addr("$(window.mira_blob||0)");
    // If the payload is not present inside window.mira_blob, 
    // set up a server to receive it.
    if (!mira_blob) {
        int q = socket(AF_INET, SOCK_STREAM, 0);
        if (q < 0) return 2;

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = 0;
        addr.sin_port = 0x3c23; // htons(9020)

        if (bind(q, &addr, sizeof(addr)) != 0) {
            close(q);
            return 3;
        }
        // listen for connections on the socket (q)
        if (listen(q, 1) != 0) {
            close(q);
            return 4;
        }
        // Accept incoming connections on the socket (q)
        int q2 = accept(q, NULL, NULL);
        if (q2 < 0) {
            close(q);
            return 5;
        }
        char* pMapped = mapping;
        int len = BLOBMAXSIZE;

        // Receive the payload data.
        while (len) {
            int bytesRead = read(q2, pMapped, len);
            if (bytesRead <= 0)
                break;

            pMapped += bytesRead;// Update position in mapped space
            len -= bytesRead;    // Update number of bytes left
        }
        // Perform cleanup of sockets
        close(q2);
        close(q);
    } else _memcpy_(mira_blob, mapping, BLOBMAXSIZE);

    int sender[512];
    pthread_create(sender, NULL, sender_thread, NULL);
    rop_call_funcptr(mapping);
    return 9;
}
