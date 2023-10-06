#include <ps4/mmap.h>
#include <librop/pthread_create.h>
#include <librop/extcall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>

#define MIRA_BLOB_SIZE 131072

// Replacement for using memcpy and when doing for-loops to copy data
// using a function makes it better looking
void _custom_memcpy(const void* _Src, void* _Dst, int _Len) {
    typedef const unsigned char* _PUCCHAR;
    typedef unsigned char* _PUCHAR;
    _PUCCHAR srcBuf = (_PUCCHAR)_Src;
    _PUCHAR  dstBuf = (_PUCHAR)_Dst;
    // Copy the contents of the source buffer into the 
    // destination byte by byte.
    for (int i = 0; i < _Len; i++)
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
    // Create a memory mapping with (read, write, and execute) 
    // permissions using mmap().
    char* mapping = mmap(NULL, MIRA_BLOB_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // Retrieve the payload data from the JS variable.
    char* mira_blob = __builtin_gadget_addr("$(window.mira_blob||0)");
    if (!mira_blob) {
        // If the payload is not present inside window.mira_blob, 
        // set up a server to receive it.
        int q = socket(AF_INET, SOCK_STREAM, 0);
        if (q < 0) return 1;
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = 0;
        addr.sin_port = 0x3c23; // htons(9020)

        bind(q, &addr, sizeof(addr));
        listen(q, 1); // listen for connections on the socket (q)

        // Accept incoming connections on the socket (q)
        int q2 = accept(q, NULL, NULL);
        char* pMapped = mapping;
        int len = MIRA_BLOB_SIZE;

        // Receive the payload data.
        while (len) {
            int bytesRead = read(q2, pMapped, len);
            if (bytesRead <= 0)
                break;

            pMapped += bytesRead;// Update position in mapped space
            len -= bytesRead;    // Update number of bytes left
        }

    } else _custom_memcpy(mira_blob, mapping, MIRA_BLOB_SIZE);

    int sender[512];
    pthread_create(sender, NULL, sender_thread, NULL);
    rop_call_funcptr(mapping);
    return 0;
}
