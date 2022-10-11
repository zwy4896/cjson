#ifndef CJSON_H__
#define CJSON_H__
#include <stdlib.h>
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
    C_PARSE_NUMBER_TOO_BIG
};

int c_parse(c_value *v, const char *json);
c_type c_get_type(const c_value *v);
double c_get_number(const c_value* v);

#endif