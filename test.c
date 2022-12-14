#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)                                                           \
    do                                                                                                             \
    {                                                                                                              \
        test_count++;                                                                                              \
        if (equality)                                                                                              \
            test_pass++;                                                                                           \
        else                                                                                                       \
        {                                                                                                          \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1;                                                                                          \
        }                                                                                                          \
    } while (0);

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define TEST_NUMBER(expect, json)                     \
    do                                                \
    {                                                 \
        c_value v;                                    \
        EXPECT_EQ_INT(C_PARSE_OK, c_parse(&v, json)); \
        EXPECT_EQ_INT(C_NUMBER, c_get_type(&v));      \
        EXPECT_EQ_DOUBLE(expect, c_get_number(&v));   \
    } while (0);

#define TEST_STRING(except, json)                                            \
    do                                                                       \
    {                                                                        \
        c_value v;                                                           \
        c_init(&v);                                                          \
        EXPECT_EQ_INT(C_PARSE_OK, c_parse(&v, json));                        \
        EXPECT_EQ_INT(C_STRING, c_get_type(&v));                             \
        EXPECT_EQ_STRING(except, c_get_string(&v), c_get_string_length(&v)); \
        c_free(&v);                                                          \
    } while (0);

#define TEST_ERROR(error, json)                  \
    do                                           \
    {                                            \
        c_value v;                               \
        c_init(&v);                              \
        v.type = C_FALSE;                        \
        EXPECT_EQ_INT(error, c_parse(&v, json)); \
        EXPECT_EQ_INT(C_NULL, c_get_type(&v));   \
        c_free(&v);                              \
    } while (0);                                 \

static void test_parse_null()
{
    c_value v;
    v.type = C_FALSE;
    EXPECT_EQ_INT(C_PARSE_OK, c_parse(&v, "null"));
    EXPECT_EQ_INT(C_NULL, c_get_type(&v));
    }

static void test_parse_string(){
    TEST_STRING("", "\"\"");
    TEST_STRING("hello", "\"hello\"");
}
static void test_parse_true(){
    c_value v;
    v.type = C_FALSE;
    EXPECT_EQ_INT(C_PARSE_OK, c_parse(&v, "true"));
    EXPECT_EQ_INT(C_TRUE, c_get_type(&v));
}

static void test_parse_false(){
    c_value v;
    v.type = C_FALSE;
    EXPECT_EQ_INT(C_PARSE_OK, c_parse(&v, "false"));
    EXPECT_EQ_INT(C_FALSE, c_get_type(&v));
}

static void test_parse_expect_value() {
    TEST_ERROR(C_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(C_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(C_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(C_PARSE_INVALID_VALUE, "?");

    #if 1
        /* invalid number */
        TEST_ERROR(C_PARSE_INVALID_VALUE, "+0");
        TEST_ERROR(C_PARSE_INVALID_VALUE, "+1");
        TEST_ERROR(C_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
        TEST_ERROR(C_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
        TEST_ERROR(C_PARSE_INVALID_VALUE, "INF");
        TEST_ERROR(C_PARSE_INVALID_VALUE, "inf");
        TEST_ERROR(C_PARSE_INVALID_VALUE, "NAN");
        TEST_ERROR(C_PARSE_INVALID_VALUE, "nan");
    #endif
}

static void test_parse_root_not_singular() {
    TEST_ERROR(C_PARSE_ROOT_NOT_SINGULAR, "null x");
    #if 1
        /* invalid number */
        TEST_ERROR(C_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
        TEST_ERROR(C_PARSE_ROOT_NOT_SINGULAR, "0x0");
        TEST_ERROR(C_PARSE_ROOT_NOT_SINGULAR, "0x123");
    #endif
}

static void test_parse_number_too_big(){
    #if 1
        TEST_ERROR(C_PARSE_NUMBER_TOO_BIG, "1e309");
        TEST_ERROR(C_PARSE_NUMBER_TOO_BIG, "-1e309");
    #endif
}

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

static void test_access_string(){
    c_value v;
    c_init(&v);
    c_set_string(&v, "", 0);
    EXPECT_EQ_STRING("", c_get_string(&v), c_get_string_length(&v));
    c_set_string(&v, "Hello", 5);
    EXPECT_EQ_STRING("Hello", c_get_string(&v), c_get_string_length(&v));
    c_free(&v);
}

static void test_parse_invalid_string_escape(){
    #if 1
    TEST_ERROR(C_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(C_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(C_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(C_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
    #endif
}

static void test_parse_invalid_string_char() {
    #if 1
    TEST_ERROR(C_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(C_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
    #endif
}

static void test_access_number(){
    c_value v;
    c_init(&v);
    c_set_string(&v, "a", 1);
    c_set_number(&v, 114.514);
    EXPECT_EQ_DOUBLE(114.514, c_get_number(&v));
    c_free(&v);
}

static void test_access_boolean(){
    c_value v;
    c_init(&v);
    c_set_string(&v, "a", 1);
    c_set_boolean(&v, 1);
    EXPECT_TRUE(c_get_boolean(&v));
    c_set_boolean(&v, 0);
    EXPECT_FALSE(c_get_boolean(&v));
    c_free(&v);
}

static void test_access_null(){
    c_value v;
    c_init(&v);
    c_set_string(&v, "a", 1);
    c_set_null(&v);
    EXPECT_EQ_INT(C_NULL, c_get_type(&v));
    c_free(&v);
}
static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_string();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}