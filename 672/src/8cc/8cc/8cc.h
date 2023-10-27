// Copyright 2012 Rui Ueyama.  
// Released under the MIT license.
// MODIFIED BY A0ZHAR
#pragma once
#ifndef EIGHTCC_H
#define EIGHTCC_H
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <time.h>

// UNTIL I've fixed the issue with nlprintf causing SyntaxError when compiling either netcat.c or jb.c
// this should be set to 0... If you want to enable printing of error msg then set it to 1
#define SHOULD_DEBUG 0 

#if SHOULD_DEBUG
// This is used to print out messages which looks like:
// [8CC] <function where this macro is called from>(): error! - <error msg ...>
#define nlprintf_e(...) \
        printf("\n[8CC] %s(): error! - ", __FUNCTION__);\
        printf(__VA_ARGS__)

// This is same as printf, however this prints the msg on a new line each time
#define nlprintf(...) printf("\n[8CC] "__VA_ARGS__)
#else
#define nlprintf_e(...)
#define nlprintf(...)
#endif

// Token Type's
#define TIDENT        0 // (Token Type) Identifier token
#define TKEYWORD      1 // (Token Type) Keyword token
#define TNUMBER       2 // (Token Type) Number token
#define TCHAR         3 // (Token Type) Character token
#define TSTRING       4 // (Token Type) String token
#define TEOF          5 // (Token Type) End of file token
#define TINVALID      6 // (Token Type) Invalid token
#define MIN_CPP_TOKEN 7 // (Token Type) Minimum token type for C preprocessor. Only in CPP
#define TNEWLINE      8 // (Token Type) Newline token
#define TSPACE        9 // (Token Type) Space token
#define TMACRO_PARAM 10 // (Token Type) Macro parameter token

// Encoding Type's
#define ENC_NONE    0 // (Encoding Type) No encoding
#define ENC_CHAR16  1 // (Encoding Type) UTF-16 encoding
#define ENC_CHAR32  2 // (Encoding Type) UTF-32 encoding
#define ENC_UTF8    3 // (Encoding Type) UTF-8 encoding
#define ENC_WCHAR   4 // (Encoding Type) Wide character encoding

// object type
#define KIND_VOID    0   // Void type
#define KIND_BOOL    1   // Boolean type
#define KIND_CHAR    2   // Character type
#define KIND_SHORT   3   // Short integer type
#define KIND_INT     4   // Integer type
#define KIND_LONG    5   // Long integer type
#define KIND_LLONG   6   // Long long integer type
#define KIND_FLOAT   7   // Float type
#define KIND_DOUBLE  8   // Double type
#define KIND_LDOUBLE 9   // Long double type
#define KIND_ARRAY   10  // Array type
#define KIND_ENUM    11  // Enumerated type
#define KIND_PTR     12  // Pointer type
#define KIND_STRUCT  13  // Struct type
#define KIND_FUNC    14  // Function type
#define KIND_STUB    15  // Used only in parser

typedef struct _Vector {
    void** body; // Pointer to the array of elements
    int len;     // Current number of elements in the vector
    int nalloc;  // Allocated capacity of the vector
} Vector;

typedef struct _Map {
    // Parent map (for hierarchical maps)
    struct _Map* parent;
    #ifdef __eir__
    Vector* v;
    #else
    char** key;  // Array of keys
    void** val;  // Array of values
    int size;    // Size of the hash table
    int nelem;   // Number of elements in the map
    int nused;   // Number of used slots in the hash table
    #endif
} Map;

typedef struct _Dict {
    struct _Map* map; // Associated map
    Vector* key;      // Vector of keys
} Dict;

typedef struct _Set {
    char* v;           // Value
    struct _Set* next; // Next element in the set
} Set;

typedef struct _Buffer {
    char* body;   // Pointer to Allocated buffer data
    int nalloc;   // Allocated capacity of the buffer
    int len;      // Current number of characters in the buffer
} Buffer;

typedef struct _File {
    FILE* file;   // File pointer (for stream-backed files)
    char* p;      // For stream backed by string
    char* name;   // File name
    int line;     // Current line number
    int column;   // Current column number
    int ntok;     // Token counter
    int last;     // The last character read from the file
    int buf[3];   // Push-back buffer for unread operations
    int buflen;   // Push-back buffer size
    time_t mtime; // Last modified time (0 for string-backed file)
} File;

typedef struct _Token {
    int kind;     // The Kind of token
    File* file;   // file stream
    int line;
    int column;
    bool space;   // true if the token has a leading space
    bool bol;     // true if the token is at the beginning of a line
    int count;    // token number in a file, counting from 0.
    Set* hideset; // used by the preprocessor for macro expansion
    union {
        int id;   // TKEYWORD
        // TSTRING or TCHAR
        struct {
            char* sval;
            int slen;
            int c;
            int enc;
        };
        // TMACRO_PARAM
        struct {
            bool is_vararg;
            int position;
        };
    };
} Token;

typedef struct _Type {
    int kind;
    int size;
    int align;
    bool usig; // true if unsigned
    bool isstatic;
    // pointer or array
    struct _Type* ptr;
    // array length
    int len;
    // struct
    Dict* fields;
    int offset;
    bool is_struct; // true if struct, false if union
    // bitfield
    int bitoff;
    int bitsize;
    // function
    struct _Type* rettype;
    Vector* params;
    bool hasva;
    bool oldstyle;
} Type;

typedef struct _SourceLoc {
    char* file;
    int line;
} SourceLoc;

typedef struct _Node {
    int kind;
    Type* ty;
    SourceLoc* sourceLoc;
    union {
        // Char, int, or long
        long ival;
        // Float or double
        struct {
            double fval;
            char* flabel;
        };
        // String
        struct {
            char* sval;
            char* slabel;
        };
        // Local/global variable
        struct {
            char* varname;
            // local
            int loff;
            Vector* lvarinit;
            // global
            char* glabel;
        };
        // Binary operator
        struct {
            struct _Node* left;
            struct _Node* right;
        };
        // Unary operator
        struct {
            struct _Node* operand;
        };
        // Function call or function declaration
        struct {
            char* fname;
            // Function call
            Vector* args;
            struct _Type* ftype;
            struct _Node* fptr;            // Function pointer or function designator
            // Function declaration
            Vector* params;
            Vector* localvars;
            struct _Node* body;
        };
        // Declaration
        struct {
            struct _Node* declvar;
            Vector* declinit;
        };
        // Initializer
        struct {
            struct _Node* initval;
            int initoff;
            Type* totype;
        };
        // If statement or ternary operator
        struct {
            struct _Node* cond;
            struct _Node* then;
            struct _Node* els;
        };
        // Goto and label
        struct {
            char* label;
            char* newlabel;
        };
        // Return statement
        struct _Node* retval;
        // Compound statement
        Vector* stmts;
        // Struct reference
        struct {
            struct _Node* struc;
            char* field;
            Type* fieldtype;
        };
    };
} Node;
// The contents of keword.inc have been added and i made sure to comment all of the enum members
// to try and improve the readability of them... 
// Please make a issue if the comments are incorrect....
enum {
    AST_LITERAL = 256,   // AST Literal node
    AST_LVAR,            // AST Local Variable node
    AST_GVAR,            // AST Global Variable node
    AST_TYPEDEF,         // AST Type Definition node
    AST_FUNCALL,         // AST Function Call node
    AST_FUNCPTR_CALL,    // AST Function Pointer Call node
    AST_FUNCDESG,        // AST Function Designator node
    AST_FUNC,            // AST Function node
    AST_DECL,            // AST Declaration node
    AST_INIT,            // AST Initialization node
    AST_CONV,            // AST Conversion node
    AST_ADDR,            // AST Address node
    AST_DEREF,           // AST Dereference node
    AST_IF,              // AST If node
    AST_TERNARY,         // AST Ternary Conditional node
    AST_DEFAULT,         // AST Default node
    AST_RETURN,          // AST Return node
    AST_COMPOUND_STMT,   // AST Compound Statement node
    AST_STRUCT_REF,      // AST Struct Reference node
    AST_GOTO,            // AST Goto node
    AST_COMPUTED_GOTO,   // AST Computed Goto node
    AST_LABEL,           // AST Label node
    OP_SIZEOF,           // Sizeof operator
    OP_CAST,             // Type casting operator
    OP_SHR,              // Shift Right operator
    OP_SHL,              // Shift Left operator
    OP_A_SHR,            // Assignment Shift Right operator
    OP_A_SHL,            // Assignment Shift Left operator
    OP_PRE_INC,          // Prefix Increment operator
    OP_PRE_DEC,          // Prefix Decrement operator
    OP_POST_INC,         // Postfix Increment operator
    OP_POST_DEC,         // Postfix Decrement operator
    OP_LABEL_ADDR,       // Label Address operator
    OP_ARROW,            // Arrow operator (->)
    OP_A_ADD,            // Assignment addition operator (+=)
    OP_A_AND,            // Assignment bitwise AND operator (&=)
    OP_A_DIV,            // Assignment division operator (/=)
    OP_A_MOD,            // Assignment modulo operator (%=)
    OP_A_MUL,            // Assignment multiplication operator (*=)
    OP_A_OR,             // Assignment bitwise OR operator (|=)
    OP_A_SAL,            // Assignment left shift operator (<<=)
    OP_A_SAR,            // Assignment right shift operator (>>=)
    OP_A_SUB,            // Assignment subtraction operator (-=)
    OP_A_XOR,            // Assignment bitwise XOR operator (^=)
    OP_DEC,              // Decrement operator (--)
    OP_EQ,               // Equality operator (==)
    OP_GE,               // Greater than or equal to operator (>=)
    OP_INC,              // Increment operator (++)
    OP_LE,               // Less than or equal to operator (<=)
    OP_LOGAND,           // Logical AND operator (&&)
    OP_LOGOR,            // Logical OR operator (||)
    OP_NE,               // Not equal operator (!=)
    OP_SAL,              // Left shift operator (<<)
    OP_SAR,              // Right shift operator (>>)
    KALIGNAS,            // _Alignas keyword
    KALIGNOF,            // _Alignof keyword
    KAUTO,               // auto keyword
    KBOOL,               // _Bool keyword
    KBREAK,              // break keyword
    KCASE,               // case keyword
    KCHAR,               // char keyword
    KCOMPLEX,            // _Complex keyword
    KCONST,              // const keyword
    KCONTINUE,           // continue keyword
    KDEFAULT,            // default keyword
    KDO,                 // do keyword
    KDOUBLE,             // double keyword
    KELSE,               // else keyword
    KENUM,               // enum keyword
    KEXTERN,             // extern keyword
    KFLOAT,              // float keyword
    KFOR,                // for keyword
    KGENERIC,            // _Generic keyword
    KGOTO,               // goto keyword
    KIF,                 // if keyword
    KIMAGINARY,          // _Imaginary keyword
    KINLINE,             // inline keyword
    KINT,                // int keyword
    KLONG,               // long keyword
    KNORETURN,           // _Noreturn keyword
    KREGISTER,           // register keyword
    KRESTRICT,           // restrict keyword
    KRETURN,             // return keyword
    KHASHHASH,           // ## operator
    KSHORT,              // short keyword
    KSIGNED,             // signed keyword
    KSIZEOF,             // sizeof keyword
    KSTATIC,             // static keyword
    KSTATIC_ASSERT,      // _Static_assert keyword
    KSTRUCT,             // struct keyword
    KSWITCH,             // switch keyword
    KELLIPSIS,           // ... ellipsis
    KTYPEDEF,            // typedef keyword
    KTYPEOF,             // typeof keyword
    KUNION,              // union keyword
    KUNSIGNED,           // unsigned keyword
    KVOID,               // void keyword
    KVOLATILE,           // volatile keyword
    KWHILE,              // while keyword
};

extern Type* type_void;
extern Type* type_bool;
extern Type* type_char;
extern Type* type_short;
extern Type* type_int;
extern Type* type_long;
extern Type* type_llong;
extern Type* type_uchar;
extern Type* type_ushort;
extern Type* type_uint;
extern Type* type_ulong;
extern Type* type_ullong;
extern Type* type_float;
extern Type* type_double;
extern Type* type_ldouble;

#define EMPTY_MAP    ((Map){})
#define EMPTY_VECTOR ((Vector){})

// from: encoding.c
Buffer* to_utf16(char* p, int len);
Buffer* to_utf32(char* p, int len);
void write_utf8(Buffer* b, uint32_t rune);

// from: buffer.c
Buffer* make_buffer(void);
char* buf_body(Buffer* b);
int buf_len(Buffer* b);
void buf_write(Buffer* b, char c);
void buf_append(Buffer* b, char* s, int len);
void buf_printf(Buffer* b, char* fmt, ...);
char* vformat(char* fmt, va_list ap);
char* format(char* fmt, ...);
char* quote_cstring(char* p);
char* quote_cstring_len(char* p, int len);
char* quote_char(char c);

// from: cpp.c
void read_from_string(char* buf);
bool is_ident(Token* tok, char* s);
void expect_newline(void);
void add_include_path(char* path);
void init_now(void);
void cpp_init(void);
Token* peek_token(void);
Token* read_token(void);

// from: debug.c
char* ty2s(Type* ty);
char* node2s(Node* node);
char* tok2s(Token* tok);

// from: dict.c
Dict* make_dict(void);
void* dict_get(Dict* dict, char* key);
void dict_put(Dict* dict, char* key, void* val);
Vector* dict_keys(Dict* dict);

// from: error.c
extern bool enable_warning;
extern bool dumpstack;
extern bool dumpsource;
extern bool warning_is_error;

#define STR2(x) #x
#define STR(x) STR2(x)
#define error(...)       errorf(__FILE__ ":" STR(__LINE__), NULL, __VA_ARGS__)
#define errort(tok, ...) errorf(__FILE__ ":" STR(__LINE__), token_pos(tok), __VA_ARGS__)
#define warn(...)        warnf(__FILE__ ":" STR(__LINE__), NULL, __VA_ARGS__)
#define warnt(tok, ...)  warnf(__FILE__ ":" STR(__LINE__), token_pos(tok), __VA_ARGS__)

noreturn void errorf(char* line, char* pos, char* fmt, ...);
void warnf(char* line, char* pos, char* fmt, ...);
char* token_pos(Token* tok);

// from: file.c
File* make_file(FILE* file, char* name);
File* make_file_string(char* s);
int readc(void);
void unreadc(int c);
File* current_file(void);
void stream_push(File* file);
int stream_depth(void);
char* input_position(void);
void stream_stash(File* f);
void stream_unstash(void);

// from: gen.c
void set_output_file(FILE* fp);
void close_output_file(void);
void emit_toplevel(Node* v);

// from: lex.c
void lex_init(char* filename);
char* get_base_file();
void skip_cond_incl(void);
char* read_header_file_name(bool* std);
bool is_keyword(Token* tok, int c);
void token_buffer_stash(Vector* buf);
void token_buffer_unstash();
void unget_token(Token* tok);
Token* lex_string(char* s);
Token* lex(void);

// from: map.c
Map* make_map(void);
Map* make_map_parent(Map* parent);
void* map_get(Map* m, char* key);
void map_put(Map* m, char* key, void* val);
void map_remove(Map* m, char* key);
size_t map_len(Map* m);

// from: parse.c
char* make_tempname(void);
char* make_label(void);
bool is_inttype(Type* ty);
bool is_flotype(Type* ty);
void* make_pair(void* first, void* second);
int eval_intexpr(Node* node, Node** addr);
Node* read_expr(void);
Vector* read_toplevels(void);
void parse_init(void);
char* fullpath(char* path);

// from: set.c
Set* set_add(Set* s, char* v);
bool set_has(Set* s, char* v);
Set* set_union(Set* a, Set* b);
Set* set_intersection(Set* a, Set* b);

// from: vector.c
Vector* make_vector(void);
Vector* make_vector1(void* e);
Vector* vec_copy(Vector* src);
void vec_push(Vector* vec, void* elem);
void vec_append(Vector* a, Vector* b);
void* vec_pop(Vector* vec);
void* vec_get(Vector* vec, int index);
void vec_set(Vector* vec, int index, void* val);
void* vec_head(Vector* vec);
void* vec_tail(Vector* vec);
Vector* vec_reverse(Vector* vec);
void* vec_body(Vector* vec);
int vec_len(Vector* vec);

#endif
