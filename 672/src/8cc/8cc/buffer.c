#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "8cc.h"

#define BUFFER_INIT_SIZE 8

// This function is not used anywhere but serves as an alternative 
// to the regular make_buffer() function
int init_new_buffer(Buffer *pBuffer) {
    // Allocate memory for our new Buffer
    pBuffer = malloc(sizeof(Buffer));

    // Check if malloc() returned NULL, indicating failure
    if (pBuffer == NULL) {
        // The reason for having a label here is to allow us to simply
        // jump inside this if statement and then assign error values
        // to the body, nalloc, and len variables before returning an error code.
        errorlbl:;
        pBuffer->nalloc = pBuffer->len = -1; // Set both nalloc and len to -1 (indicating an error)
        pBuffer->body = NULL; // Set the body to NULL (indicating an error)
        return -1; // Return an error code
    }

    // Otherwise, we allocate memory (initialized to 0) for our buffer's body
    pBuffer->body = calloc(1, BUFFER_INIT_SIZE);
    if (pBuffer->body == NULL) {
        free(pBuffer); // Free the memory previously allocated for the buffer
        goto errorlbl; // Then jump to the part where we assign error values to buffer members
    }

    // If memory allocation using malloc for the buffer itself
    // and calloc for the body both succeeded, we finalize by setting
    // the maximum number of characters (nalloc) to 8 and len (length) to 0.
    pBuffer->nalloc = BUFFER_INIT_SIZE;
    pBuffer->len = 0;
    return 1; // Return a success value
}


Buffer *make_buffer() {
    simplelogging("making buffer");
    // Create a new Buffer instance
    Buffer *buf = malloc(sizeof(Buffer));
    if (buf == NULL) {
        simplelogging("malloc failed");
        return NULL;
    }
    // Allocate memory for the body of buffer
    char *body = malloc(BUFFER_INIT_SIZE);
    if (body == NULL) {
        simplelogging("Failed to allocate memory for the body");
        free(buf);// free memory allocated for buf
        return NULL;
    }
    buf->body = body;
    buf->nalloc = BUFFER_INIT_SIZE;
    buf->len = 0;
    return buf;
}

int resizeBuffer(Buffer *buf) {
    if (buf == NULL || buf->body == NULL) {
        simplelogging("buffer or its body passed is NULL");
        return -1;
    }

    char *newbody = realloc(buf->body, buf->nalloc * 2);
    if (newbody == NULL) {
        simplelogging("realloc failed!");
        return -1;
    }

    buf->body = newbody;  // Assign the new memory (thats twice as big as previous)
    buf->nalloc *= 2;     // Double capacity
    return 1;
}

char *buf_body(Buffer *b) {
    return b->body;
}

int buf_len(Buffer *b) {
    return b->len;
}

// Writes a single character to the body of a buffer
void buf_write(Buffer *b, char c) {
    if (b == NULL || b->body == NULL) {
        simplelogging("error! buffer or it's body is NULL");
        return;
    }
    // If max capacity is equal to current length of the 
    // body + 1 additionall character
    if (b->nalloc <= b->len + 1)
        if (resizeBuffer(b) == -1)
            return;

    // copy character into body
    b->body[b->len++] = c;
}

void buf_append(Buffer *b, char *data, int len) {
    if (b == NULL || b->body == NULL) {
        simplelogging("error! buffer or it's body is NULL");
        return;
    }

    // Resize the memory until new data fits
    // original cond: avail <= written
    while (b->nalloc <= b->len + len) {
        if (resizeBuffer(b) == -1)
            return;
    }

    // Copy all data at once
    memcpy(b->body + b->len, data, len);
    b->len += len; //update length
}


void buf_printf(Buffer *b, char *fmt, ...) {
    if (b == NULL) {
        simplelogging("buff, or it's body is NULL");
        return;
    }
    va_list tmpargs;
    va_start(tmpargs, fmt);
    int fmtlen = vsnprintf(NULL, 0, fmt, tmpargs) + 1;
    va_end(tmpargs);

    // Resize the memory until new data fits
    // original cond: avail <= written
    while ((b->nalloc - b->len) <= fmtlen)
        if (resizeBuffer(b) == -1)
            return;

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(b->body + b->len, fmtlen, fmt, args);
    va_end(args);
    b->len += written;
}

char *vformat(char *fmt, va_list ap) {
    Buffer *b = make_buffer();
    if (b == NULL) {
        simplelogging("make_buffer() failed!");
        return NULL;
    }

    va_list tmpargs;
    va_copy(tmpargs, ap);
    // Obtain the length of our string after being formatted
    int fmtlen = vsnprintf(NULL, 0, fmt, tmpargs) + 1;
    va_end(tmpargs);

    // Resize the memory until new data fits
    // original cond: avail <= written
    while ((b->nalloc - b->len) <= fmtlen)
        if (resizeBuffer(b) == -1)
            return NULL;

    va_list aq;
    va_copy(aq, ap);
    // copy formatted string into the body of our buffer.
    int written = vsnprintf(b->body + b->len, fmtlen, fmt, ap);
    va_end(aq);
    b->len += written;
    return buf_body(b);
}
// issues with this is that it creates a new buffer instance
// and returns the body of that buffer instance, after copying
// formatted string to it... 
// But the buffer isnt deallocate, and can't be deallocated 
// using current format implementation
char *format(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *r = vformat(fmt, ap);
    va_end(ap);
    return r;
}

static char *quote(char c) {
    switch (c) {
        case '"':  return "\\\"";
        case '\\': return "\\\\";
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        default: /* unknown/none of above*/ break;
    };
    return NULL;
}

static void print(Buffer *b, char c) {
    char *q = quote(c);
    if (q) {
        buf_printf(b, "%s", q);
    } else if (isprint(c)) {
        buf_printf(b, "%c", c);
    } else {
        #ifdef __eir__
        buf_printf(b, "\\x%x", c);
        #else
        buf_printf(b, "\\x%02x", ((int)c) & 255);
        #endif
    }
}

char *quote_cstring(char *p) {
    Buffer *b = make_buffer();
    if (b == NULL) {
        simplelogging("make_buffer() failed!");
        return NULL;
    }

    while (*p) print(b, *p++);

    return buf_body(b);
}

char *quote_cstring_len(char *p, int len) {
    Buffer *b = make_buffer();
    if (b == NULL) {
        simplelogging("make_buffer() failed!");
        return NULL;
    }
    for (int i = 0; i < len; i++)
        print(b, p[i]);

    return buf_body(b);
}

char *quote_char(char c) {
    if (c == '\\') return "\\\\";
    if (c == '\'') return "\\'";
    return format("%c", c);
}
