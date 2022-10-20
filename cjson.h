#ifndef CJSON_H__
#define CJSON_H__
#include <stdlib.h>

#define c_init(v) do { (v)->type = C_NULL; } while(0)
#define c_set_null(v) c_free(v)
typedef enum
{
    C_NULL,
    C_FALSE,
    C_TRUE,
    C_NUMBER,
    C_STRING,
    C_ARRAY,
    C_OBJECT
} c_type;

typedef struct
{
    union
    {
        struct{char* s; size_t len;};
        double n;
    };
    c_type type;
}c_value;

enum
{
    C_PARSE_OK = 0,
    C_PARSE_EXPECT_VALUE,
    C_PARSE_INVALID_VALUE,
    C_PARSE_ROOT_NOT_SINGULAR,
    C_PARSE_NUMBER_TOO_BIG,
    C_PARSE_MISS_QUOTATION_MARK
};

int c_parse(c_value *v, const char *json);
c_type c_get_type(const c_value *v);
double c_get_number(const c_value* v);
void c_free(c_value *v);
int c_get_boolean(const c_value* v);
void c_set_boolean(c_value* v, int b);

void c_set_number(c_value* v, double n);

const char* c_get_string(const c_value* v);
size_t c_get_string_length(const c_value* v);
void c_set_string(c_value* v, const char* s, size_t len);

#endif