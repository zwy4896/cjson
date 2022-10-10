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

static int c_parse_null(c_context* c, c_value* v)
{
    EXCEPT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return C_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = C_NULL;
    return C_PARSE_OK;
}

static int c_parse_true(c_context* c, c_value* v){
    EXCEPT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return C_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = C_TRUE;
    return C_PARSE_OK;
}

static int c_parse_false(c_context* c, c_value* v){
    EXCEPT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return C_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = C_FALSE;
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
        return c_parse_null(c, v);
    case 't':
        return c_parse_true(c, v);
    case 'f':
        return c_parse_false(c, v);
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
