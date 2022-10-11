#include "cjson.h"
#include <assert.h>
#include <stdlib.h>

#define EXCEPT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0);

typedef struct 
{
    const char *json;
}c_context;

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
    char* end;
    /* \TODO validate number */
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return C_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = C_NUMBER;
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
    v->type = C_NULL;
    c_parse_whitespace(&c);
    if((ret = c_parse_value(&c, v)) == C_PARSE_OK){
        c_parse_whitespace(&c);
        if (*c.json != '\0'){
            v->type = C_NULL;
            ret = C_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

c_type c_get_type(const c_value* v)
{
    assert(v != NULL);
    return v->type;
}

double c_get_number(const c_value* v){
    assert(v != NULL && v->type == C_NUMBER);
    return v->n;
}
