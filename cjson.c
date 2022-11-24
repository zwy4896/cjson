#include "cjson.h"
#include <assert.h>
#include <math.h>
#include <string.h>

#ifndef C_PARSE_STACK_INIT_SIZE
#define C_PARSE_STACK_INIT_SIZE 256
#endif

#define EXCEPT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0);

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do { *(char*)c_context_push(c, sizeof(char)) = (ch); } while(0)

typedef struct
{
    const char *json;
    char *stack;
    size_t size, top;
} c_context;

static void* c_context_push(c_context* c, size_t size){
    void *ret;
    assert(size > 0);
    if(c->top+size >= c->size){
        if (c->size == 0)
            c->size = C_PARSE_STACK_INIT_SIZE;
        while (c->top+size >= c->size)
        {
            c->size += c->size >> 1;
        }
        c->stack = (char *)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* c_context_pop(c_context* c, size_t size){
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static int c_parse_string(c_context* c, c_value* v){
    size_t head = c->top, len;
    const char *p;
    EXCEPT(c, '\"');
    p = c->json;
    for (;;)
    {
        char ch = *p++;
        switch (ch)
        {
        case '\"':
            len = c->top - head;
            c_set_string(v, (const char *)c_context_pop(c, len), len);
            c->json = p;
            return C_PARSE_OK;
        case '\\':
        switch (*p++)
        {
        case '\"':
            PUTC(c, '\"');break;
        case '\\':
            PUTC(c, '\\'); break;
        case '/':
            PUTC(c, '/'); break;
        case 'b':
            PUTC(c, 'b'); break;
        case 'f':
            PUTC(c, 'f'); break;
        case 'n':
            PUTC(c, 'n');break;
        case 'r':
            PUTC(c, 'r');break;
        case 't':
            PUTC(c, 't');break;
        default:
            c->top = head;
            return C_PARSE_INVALID_STRING_ESCAPE;
        }
        case '\0':
            c->top = head;
            return C_PARSE_MISS_QUOTATION_MARK;
        default:
        if((unsigned char) ch < 0x20){
                c->top = head;
                return C_PARSE_INVALID_STRING_CHAR;
            }
            PUTC(c, ch);
        }
    }
}
static void c_parse_whitespace(c_context* c)
{
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    {
        p++;
    }
    c->json = p;
}

static int c_parse_literal(c_context* c, c_value* v, const char* literal, c_type type){
    size_t i;
    EXCEPT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
    if(c->json[i] != literal[i+1])
        return C_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return C_PARSE_OK;
}

static int c_parse_number(c_context* c, c_value* v) {
    const char *p = c->json;
    if(*p == '-')
        p++;
    if(*p == '0')
        p++;
    else{
        if(!ISDIGIT1TO9(*p))
            return C_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ;
    }
    if(*p == '.'){
        p++;
        if(!ISDIGIT(*p))
            return C_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ;
    }
    if(*p == 'e' || *p == 'E'){
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return C_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ;
    }
    v->n = strtod(c->json, NULL);
    if (v->n == HUGE_VAL || v->n == -HUGE_VAL)
        return C_PARSE_NUMBER_TOO_BIG;
    v->type = C_NUMBER;
    c->json = p;

    return C_PARSE_OK;
}

static int c_parse_value(c_context* c, c_value* v)
{
    switch (*c->json)
    {
    case 'n':
        return c_parse_literal(c, v, "null", C_NULL);
    case 't':
        return c_parse_literal(c, v, "true", C_TRUE);
    case 'f':
        return c_parse_literal(c, v, "false", C_FALSE);
    case '\0':
        return C_PARSE_EXPECT_VALUE;
    case '"':
        return c_parse_string(c, v);
    default:
        return c_parse_number(c, v);
    }
}

int c_parse(c_value* v, const char* json)
{
    c_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    c_init(v);
    c_parse_whitespace(&c);
    if((ret = c_parse_value(&c, v)) == C_PARSE_OK){
        c_parse_whitespace(&c);
        if (*c.json != '\0'){
            v->type = C_NULL;
            ret = C_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

c_type c_get_type(const c_value* v)
{
    assert(v != NULL);
    return v->type;
}

int c_get_boolean(const c_value* v){
    assert(v != NULL && (v->type == C_TRUE || v->type == C_FALSE));
    return v->type == C_TRUE;
}

double c_get_number(const c_value* v){
    assert(v != NULL && v->type == C_NUMBER);
    return v->n;
}

void c_free(c_value* v){
    assert(v!= NULL);
    if(v->type == C_STRING)
        free(v->s);
    v->type = C_NULL;
}

void c_set_string(c_value* v, const char* s, size_t len){
    assert(v != NULL && (s != NULL || len == 0));
    c_free(v);
    v->s = (char*)malloc(len + 1);
    memcpy(v->s, s, len);
    v->s[len] = '\0';
    v->len = len;
    v->type = C_STRING;
}

void c_set_number(c_value* v, double n){
    c_free(v);
    v->n = n;
    v->type = C_NUMBER;
}

void c_set_boolean(c_value*v, int b){
    c_free(v);
    v->type = b ? C_TRUE : C_FALSE;
}
size_t c_get_string_length(const c_value* v) {
    assert(v != NULL && v->type == C_STRING);
    return v->len;
}

const char* c_get_string(const c_value* v) {
    assert(v != NULL && v->type == C_STRING);
    return v->s;
}