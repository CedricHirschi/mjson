// Copyright (c) 2018-2020 Cesanta Software Limited
// All rights reserved

#include <float.h>  // For DBL_EPSILON
#include <math.h>
#include <stdio.h>  // For printf

#include "mjson.h"

static int s_num_tests = 0;
static int s_num_errors = 0;

#define ASSERT(expr)                                              \
  do {                                                            \
    s_num_tests++;                                                \
    if (!(expr)) {                                                \
      s_num_errors++;                                             \
      printf("\n  FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);  \
        printf("%-26s", " "); \
    }                                                             \
  } while (0)

#define ASSERT_EQ_STR(a, b) \
  do { \
    s_num_tests++; \
    size_t _n_a = strlen(a); \
    size_t _n_b = strlen(b); \
    size_t _n = (_n_a < _n_b) ? _n_a : _n_b; \
    if (strncmp(a, b, _n) != 0) { \
      s_num_errors++; \
      printf("\n  FAIL %s:%d: Strings do not match ('%s' != '%s')\n", __FILE__, __LINE__, a, b); \
      printf("%-26s", " "); \
    } \
  } while (0)

#define ASSERT_EQ(a, b) \
  do { \
    s_num_tests++; \
    if ((a) != (b)) { \
      s_num_errors++; \
      printf("\n  FAIL %s:%d: Values do not match (%d != %d)\n", __FILE__, __LINE__, a, b); \
      printf("%-26s", " "); \
    } \
  } while (0)

#define RUN_SUITE(fn) \
  do { \
    printf("%-24s  ", #fn);   \
    int s_num_errors_prev = s_num_errors; \
    fn(); \
    if (s_num_errors != s_num_errors_prev) { \
      printf("%4d  %4d\r\n", s_num_tests, s_num_errors - s_num_errors_prev); \
    } else { \
      printf("%4d     -\r\n", s_num_tests); \
    } \
  } while (0)

#ifdef _WIN32
#define snprintf _snprintf
#endif

static void test_cb(void) {
  const char *str;
  {
    const char *s = "{\"a\": true, \"b\": [ null, 3 ]}";
    ASSERT_EQ(mjson(s, (int) strlen(s), NULL, NULL), (int) strlen(s));
  }
  {
    const char *s = "[ 1, 2 ,  null, true,false,\"foo\"  ]";
    ASSERT_EQ(mjson(s, (int) strlen(s), NULL, NULL), (int) strlen(s));
  }
  {
    const char *s = "123";
    ASSERT_EQ(mjson(s, (int) strlen(s), NULL, NULL), (int) strlen(s));
  }
  {
    const char *s = "\"foo\"";
    ASSERT_EQ(mjson(s, (int) strlen(s), NULL, NULL), (int) strlen(s));
  }
  {
    const char *s = "123 ";  // Trailing space
    ASSERT_EQ(mjson(s, (int) strlen(s), NULL, NULL), (int) strlen(s) - 1);
  }
  {
    const char *s = "[[[[[[[[[[[[[[[[[[[[[";
    ASSERT_EQ(mjson(s, (int) strlen(s), NULL, NULL), MJSON_ERROR_TOO_DEEP);
  }

  str = "\"abc\"";
  ASSERT_EQ(mjson(str, 0, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 1, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 2, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 3, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 4, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 5, NULL, NULL), 5);

  str = "{\"a\":1}";
  ASSERT_EQ(mjson(str, 0, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 1, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 2, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 3, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 4, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 5, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 6, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
  ASSERT_EQ(mjson(str, 7, NULL, NULL), 7);

  str = "{\"a\":[]}";
  ASSERT_EQ(mjson(str, 8, NULL, NULL), 8);

  str = "{\"a\":{}}";
  ASSERT_EQ(mjson(str, 8, NULL, NULL), 8);
  ASSERT_EQ(mjson("[]", 2, NULL, NULL), 2);
  ASSERT_EQ(mjson("{}", 2, NULL, NULL), 2);
  ASSERT_EQ(mjson("[[]]", 4, NULL, NULL), 4);
  ASSERT_EQ(mjson("[[],[]]", 7, NULL, NULL), 7);
  ASSERT_EQ(mjson("[{}]", 4, NULL, NULL), 4);
  ASSERT_EQ(mjson("[{},{}]", 7, NULL, NULL), 7);

  str = "{\"a\":[{}]}";
  ASSERT_EQ(mjson(str, 10, NULL, NULL), 10);

  ASSERT_EQ(mjson("]", 1, NULL, NULL), MJSON_ERROR_INVALID_INPUT);
}

static void test_find(void) {
  const char *p, *str;
  int n;
  ASSERT_EQ(mjson_find("", 0, "", &p, &n), MJSON_TOK_INVALID);
  ASSERT_EQ(mjson_find("", 0, "$", &p, &n), MJSON_TOK_INVALID);
  ASSERT_EQ(mjson_find("123", 3, "$", &p, &n), MJSON_TOK_NUMBER);
  ASSERT_EQ(n, 3);
  ASSERT_EQ(memcmp(p, "123", 3), 0);

  str = "{\"a\":true}";
  ASSERT_EQ(mjson_find(str, 10, "$.a", &p, &n), MJSON_TOK_TRUE);
  ASSERT_EQ(n, 4);
  ASSERT_EQ_STR(p, "true");

  str = "{\"a\":{\"c\":null},\"c\":2}";
  ASSERT_EQ(mjson_find(str, 22, "$.c", &p, &n), MJSON_TOK_NUMBER);
  ASSERT_EQ(n, 1);
  ASSERT_EQ_STR(p, "2");

  str = "{\"a\":{\"c\":null},\"c\":2}";
  ASSERT_EQ(mjson_find(str, 22, "$.a.c", &p, &n), MJSON_TOK_NULL);
  ASSERT_EQ(n, 4);
  ASSERT_EQ_STR(p, "null");

  str = "{\"a\":[1,null]}";
  ASSERT_EQ(mjson_find(str, 15, "$.a", &p, &n), '[');
  ASSERT_EQ(n, 8);
  ASSERT_EQ_STR(p, "[1,null]");

  str = "{\"a\":{\"b\":7}}";
  ASSERT_EQ(mjson_find(str, 14, "$.a", &p, &n), '{');

  str = "{\"b\":7}";
  ASSERT_EQ(n, 7);
  ASSERT_EQ_STR(p, str);

  // Test the shortcut: as soon as we find an element, stop the traversal
  str = "{\"a\":[1,2,garbage here!!";
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a[0]", &p, &n), MJSON_TOK_NUMBER);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a[1]", &p, &n), MJSON_TOK_NUMBER);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a[2]", &p, &n), MJSON_TOK_INVALID);

  // Test array iteration
  str = "{\"a\":[1,2],\"b\":[3,4,5,6]}";
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a[0]", &p, &n), MJSON_TOK_NUMBER);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a[1]", &p, &n), MJSON_TOK_NUMBER);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a[2]", &p, &n), MJSON_TOK_INVALID);

  str = "{\"a1\":[{\"x\":1},{\"x\":2}],\"a2\":[{\"x\":3},{\"x\":4}]}";
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a1[0].x", &p, &n),
            MJSON_TOK_NUMBER);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a1[1].x", &p, &n),
            MJSON_TOK_NUMBER);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a1[2].x", &p, &n),
            MJSON_TOK_INVALID);

  str = "{\"a.b\":{\"c\":1}}";
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a.b", &p, &n),
            MJSON_TOK_INVALID);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a\\.b", &p, &n),
            MJSON_TOK_OBJECT);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.a\\.b.c", &p, &n),
            MJSON_TOK_NUMBER);
  ASSERT_EQ(n, 1);
  ASSERT_EQ(*p, '1');

  str = "{\"[]\":1}";
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.[]", &p, &n),
            MJSON_TOK_INVALID);
  ASSERT_EQ(mjson_find(str, (int) strlen(str), "$.\\[\\]", &p, &n),
            MJSON_TOK_NUMBER);
  ASSERT_EQ(n, 1);
  ASSERT_EQ(*p, '1');

  {
    const char *s = "{\"a\":[{\"b\":1},{\"b\":2,\"c\":3}]}";
    int len = (int) strlen(s);
    ASSERT_EQ(mjson_find(s, len, "$.a[0].b", &p, &n), MJSON_TOK_NUMBER);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(*p, '1');
    ASSERT_EQ(mjson_find(s, len, "$.a[1].b", &p, &n), MJSON_TOK_NUMBER);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(*p, '2');
    ASSERT_EQ(mjson_find(s, len, "$.a[1].c", &p, &n), MJSON_TOK_NUMBER);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(*p, '3');
    ASSERT_EQ(mjson_find(s, len, "$.a[0].c", &p, &n), MJSON_TOK_INVALID);
    ASSERT_EQ(mjson_find(s, len, "$.a[0]", &p, &n), MJSON_TOK_OBJECT);
    ASSERT_EQ(mjson_find(s, len, "$.a", &p, &n), MJSON_TOK_ARRAY);
  }

  {
    const char *s = "{\"a\":[]}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_ARRAY);
    s = "{\"a\":[1,2]}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_ARRAY);
    s = "{\"a\":[1,[1]]}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_ARRAY);
    s = "{\"a\":[[]]}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_ARRAY);
    s = "{\"a\":[[1,2]]}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_ARRAY);
    s = "{\"a\":{}}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_OBJECT);
    s = "{\"a\":{\"a\":{}}}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_OBJECT);
    s = "{\"a\":{\"a\":[]}}";
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a", &p, &n), MJSON_TOK_OBJECT);
  }
}

// Compare two double numbers
static int eqdbl(double a, double b) {
  return (a - b < DBL_EPSILON) && (b - a < DBL_EPSILON);
}

static void test_get_number(void) {
  const char *str;
  double v;
  ASSERT_EQ(mjson_get_number("", 0, "$", &v), 0);
  ASSERT(mjson_get_number("234", 3, "$", &v) == 1 && v == 234);
  str = "{\"a\":-7}";
  ASSERT(mjson_get_number(str, 8, "$.a", &v) == 1 && v == -7);
  ASSERT(mjson_get_number("1.2e3", 8, "$", &v) == 1 && v == 1.2e3);
  ASSERT(mjson_get_number("-0.00013", 8, "$", &v) == 1 && eqdbl(v, -0.00013));
  ASSERT(mjson_get_number("-0.123456", 9, "$", &v) == 1 && eqdbl(v, -0.123456));
  ASSERT(mjson_get_number("99.999999", 9, "$", &v) == 1 && v == 99.999999);
  ASSERT(mjson_get_number("1.23e+12", 8, "$", &v) == 1 && v == 1.23e+12);
  ASSERT(mjson_get_number("1.23e-44", 8, "$", &v) == 1 && eqdbl(v, 1.23e-44));
  // printf("==> [%g]\n", v);
  ASSERT_EQ(mjson_get_number("[1.23,-43.47,17]", 16, "$", &v), 0);
  ASSERT(mjson_get_number("[1.23,-43.47,17]", 16, "$[0]", &v) == 1 &&
         v == 1.23);
  ASSERT(mjson_get_number("[1.23,-43.47,17]", 16, "$[1]", &v) == 1 &&
         v == -43.47);
  ASSERT(mjson_get_number("[1.23,-43.47,17]", 16, "$[2]", &v) == 1 && v == 17);
  ASSERT_EQ(mjson_get_number("[1.23,-43.47,17]", 16, "$[3]", &v), 0);
  {
    const char *s = "{\"a1\":[1,2,{\"a2\":4},[],{}],\"a\":3}";
    ASSERT(mjson_get_number(s, (int) strlen(s), "$.a", &v) == 1 && v == 3);
  }
  {
    const char *s = "[1,{\"a\":2}]";
    ASSERT(mjson_get_number(s, (int) strlen(s), "$[0]", &v) == v && v == 1);
    ASSERT(mjson_get_number(s, (int) strlen(s), "$[1].a", &v) == 1 && v == 2);
  }
  ASSERT(mjson_get_number("[[2,1]]", 7, "$[0][1]", &v) == 1 && v == 1);
  ASSERT(mjson_get_number("[[2,1]]", 7, "$[0][0]", &v) == 1 && v == 2);
  ASSERT(mjson_get_number("[[2,[]]]", 8, "$[0][0]", &v) == 1 && v == 2);
  ASSERT(mjson_get_number("[1,[2,[]]]", 10, "$[1][0]", &v) == 1 && v == 2);
  ASSERT(mjson_get_number("[{},1]", 6, "$[1]", &v) == 1 && v == 1);
  ASSERT(mjson_get_number("[[],1]", 6, "$[1]", &v) == 1 && v == 1);
  ASSERT(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[0]", &v) == 1 && v == 1);
  ASSERT_EQ(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1]", &v), 0);
  ASSERT(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1][0]", &v) == 1 &&
         v == 2);
  ASSERT(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1][2]", &v) == 1 &&
         v == 3);
  ASSERT(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1][3][0]", &v) == 1 &&
         v == 4);
  ASSERT(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1][3][1]", &v) == 1 &&
         v == 5);
  ASSERT_EQ(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1][3][2]", &v), 0);
  ASSERT_EQ(mjson_get_number("[1,[2,[],3,[4,5]]]", 18, "$[1][3][2][0]", &v), 0);

  str = "[1,2,{\"a\":[3,4]}]";
  ASSERT(mjson_get_number(str, 17, "$[1]", &v) == 1 && v == 2);
  str = "[1,2,{\"a\":[3,4]}]";
  ASSERT(mjson_get_number(str, 17, "$[2].a[0]", &v) == 1 && v == 3);
  str = "[1,2,{\"a\":[3,4]}]";
  ASSERT(mjson_get_number(str, 17, "$[2].a[1]", &v) == 1 && v == 4);
  str = "[1,2,{\"a\":[3,4]}]";
  ASSERT_EQ(mjson_get_number(str, 17, "$[2].a[2]", &v), 0);
  str = "{\"a\":3,\"ab\":2}";
  ASSERT(mjson_get_number(str, 14, "$.ab", &v) == 1 && v == 2);
}

static void test_get_bool(void) {
  const char *s = "{\"state\":{\"lights\":true,\"version\":36,\"a\":false}}";
  double x;
  int v;
  ASSERT_EQ(mjson_get_bool("", 0, "$", &v), 0);
  ASSERT_EQ(mjson_get_bool("true", 4, "$", &v), 1);
  ASSERT_EQ(v, 1);
  ASSERT_EQ(mjson_get_bool("false", 5, "$", &v), 1);
  ASSERT_EQ(v, 0);
  ASSERT_EQ(mjson_get_number(s, (int) strlen(s), "$.state.version", &x), 1);
  ASSERT(x == 36);
  ASSERT_EQ(mjson_get_bool(s, (int) strlen(s), "$.state.a", &v), 1);
  ASSERT_EQ(v, 0);
  ASSERT_EQ(mjson_get_bool(s, (int) strlen(s), "$.state.lights", &v), 1);
  ASSERT_EQ(v, 1);
}

static void test_get_string(void) {
  char buf[100];
  {
    const char *s = "{\"a\":\"f\too\"}";
    ASSERT_EQ(mjson_get_string(s, (int) strlen(s), "$.a", buf, sizeof(buf)), 4);
    ASSERT_EQ_STR(buf, "f\too");
  }

  {
    const char *s = "{\"ы\":\"превед\"}";
    ASSERT_EQ(mjson_get_string(s, (int) strlen(s), "$.ы", buf, sizeof(buf)), 12);
    ASSERT_EQ_STR(buf, "превед");
  }

  {
    const char *s = "{\"a\":{\"x\":\"X\"},\"b\":{\"q\":\"Y\"}}";
    ASSERT_EQ(mjson_get_string(s, (int) strlen(s), "$.a.x", buf, sizeof(buf)),
              1);
    ASSERT_EQ_STR(buf, "X");
    ASSERT_EQ(mjson_find(s, (int) strlen(s), "$.a.q", NULL, NULL),
              MJSON_TOK_INVALID);
    ASSERT(mjson_get_string(s, (int) strlen(s), "$.a.q", buf, sizeof(buf)) < 0);
    // printf("-->[%s]\n", buf);
    ASSERT_EQ(mjson_get_string(s, (int) strlen(s), "$.b.q", buf, sizeof(buf)),
              1);
    ASSERT_EQ_STR(buf, "Y");
  }

  {
    struct {
      char buf1[3], buf2[3];
    } foo;
    const char *s = "{\"a\":\"0123456789\"}";
    memset(&foo, 0, sizeof(foo));
    ASSERT_EQ(mjson_get_string(s, (int) strlen(s), "$.a", foo.buf1,
                               sizeof(foo.buf1)), -1);
    ASSERT_EQ(foo.buf1[0], '0');
    ASSERT_EQ(foo.buf1[2], '2');
    ASSERT_EQ(foo.buf2[0], '\0');
  }

  {
    const char *s = "[\"MA==\",\"MAo=\",\"MAr+\",\"MAr+Zw==\"]";
    ASSERT_EQ(mjson_get_base64(s, (int) strlen(s), "$[0]", buf, sizeof(buf)), 1);
    ASSERT_EQ_STR(buf, "0");
    ASSERT_EQ(mjson_get_base64(s, (int) strlen(s), "$[1]", buf, sizeof(buf)), 2);
    ASSERT_EQ_STR(buf, "0\n");
    ASSERT_EQ(mjson_get_base64(s, (int) strlen(s), "$[2]", buf, sizeof(buf)), 3);
    ASSERT_EQ_STR(buf, "0\n\xfe");
    ASSERT_EQ(mjson_get_base64(s, (int) strlen(s), "$[3]", buf, sizeof(buf)), 4);
    ASSERT_EQ_STR(buf, "0\n\xfeg");
  }

  {
    const char *s = "[\"200a\",\"fe31\",123]";
    ASSERT_EQ(mjson_get_hex(s, (int) strlen(s), "$[0]", buf, sizeof(buf)), 2);
    ASSERT_EQ_STR(buf, " \n");
    ASSERT_EQ(mjson_get_hex(s, (int) strlen(s), "$[1]", buf, sizeof(buf)), 2);
    ASSERT_EQ_STR(buf, "\xfe\x31");
    ASSERT(mjson_get_hex(s, (int) strlen(s), "$[2]", buf, sizeof(buf)) < 0);
  }

  {
    const char *s = "[1,2]";
    double dv;
    ASSERT(mjson_get_number(s, (int) strlen(s), "$[0]", &dv) == 1 && dv == 1);
    ASSERT(mjson_get_number(s, (int) strlen(s), "$[1]", &dv) == 1 && dv == 2);
    ASSERT_EQ(mjson_get_number(s, (int) strlen(s), "$[3]", &dv), 0);
  }

  {
    const char *s = "[1,2,\"hello \\u0026\\u003c\\u003e\\\"\"]";
    const char *expected = "hello &<>\"";
    ASSERT(mjson_get_string(s, (int) strlen(s), "$[2]", buf, sizeof(buf)) > 0);
    ASSERT_EQ_STR(buf, expected);
  }
}

static void test_print(void) {
  char tmp[100];
  const char *str;

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, -97, 1), 3);
    ASSERT(memcmp(tmp, "-97", 3) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, -97, 0), 10);
    ASSERT(memcmp(tmp, "4294967199", 10) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, -97, 1), 3);
    ASSERT(memcmp(tmp, "-97", 3) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, 2, 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, -97, 1), 1);
    ASSERT_EQ(fb.len, fb.size - 1);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, 0, 1), 1);
    // printf("-->[%s]\n", tmp);
    ASSERT(memcmp(tmp, "0", 1) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, 12345678, 1), 8);
    ASSERT(memcmp(tmp, "12345678", 8) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_int(&mjson_print_fixed_buf, &fb, (int) 3456789012, 0),
              10);
    ASSERT(memcmp(tmp, "3456789012", 10) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_print_str(&mjson_print_fixed_buf, &fb, "a", 1), 3);
    str = "\"a\"";
    ASSERT(memcmp(tmp, str, 3) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    const char *s = "a\b\n\f\r\t\"";
    ASSERT_EQ(mjson_print_str(&mjson_print_fixed_buf, &fb, s, 7), 15);
    str = "\"a\\b\\n\\f\\r\\t\\\"\"";
    ASSERT(memcmp(tmp, str, 15) == 0);
    ASSERT(fb.len < fb.size);
  }
}

static int f1(mjson_print_fn_t fn, void *fndata, va_list *ap) {
  int value = va_arg(*ap, int);
  return mjson_printf(fn, fndata, "[%d]", value);
}

static void test_printf(void) {
  const char *str;
  char tmp[100];

#define DBLWIDTH(a, b) a, b
#define TESTDOUBLE(fmt_, num_, res_)                                    \
  do {                                                                  \
    const char *N = #num_;                                              \
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};                   \
    int n = mjson_printf(&mjson_print_fixed_buf, &fb, fmt_, num_);      \
    if (0) printf("[%s] [%s] -> [%s] [%.*s]\n", fmt_, N, res_, n, tmp); \
    ASSERT_EQ(n, (int) strlen(res_));                                   \
    ASSERT(strncmp(tmp, res_, (size_t) n) == 0);                        \
  } while (0)

  TESTDOUBLE("%g", 0.0, "0");
  TESTDOUBLE("%g", 0.123, "0.123");
  TESTDOUBLE("%g", 0.00123, "0.00123");
  TESTDOUBLE("%g", 0.123456333, "0.123456");
  TESTDOUBLE("%g", 123.0, "123");
  TESTDOUBLE("%g", 11.5454, "11.5454");
  TESTDOUBLE("%g", 11.0001, "11.0001");
  TESTDOUBLE("%g", 0.999, "0.999");
  TESTDOUBLE("%g", 0.999999, "0.999999");
  TESTDOUBLE("%g", 0.9999999, "1");
  TESTDOUBLE("%g", 10.9, "10.9");
  TESTDOUBLE("%g", 10.01, "10.01");
  TESTDOUBLE("%g", 1.0, "1");
  TESTDOUBLE("%g", 10.0, "10");
  TESTDOUBLE("%g", 100.0, "100");
  TESTDOUBLE("%g", 1000.0, "1000");
  TESTDOUBLE("%g", 10000.0, "10000");
  TESTDOUBLE("%g", 100000.0, "100000");
  TESTDOUBLE("%g", 1000000.0, "1e+06");
  TESTDOUBLE("%g", 10000000.0, "1e+07");
  TESTDOUBLE("%g", 100000001.0, "1e+08");
  TESTDOUBLE("%g", 10.5454, "10.5454");
  TESTDOUBLE("%g", 999999.0, "999999");
  TESTDOUBLE("%g", 9999999.0, "1e+07");
  TESTDOUBLE("%g", 44556677.0, "4.45567e+07");
  TESTDOUBLE("%g", 1234567.2, "1.23457e+06");
  TESTDOUBLE("%g", -987.65432, "-987.654");
  TESTDOUBLE("%g", 0.0000000001, "1e-10");
  TESTDOUBLE("%g", 2.34567e-57, "2.34567e-57");
  TESTDOUBLE("%.*g", DBLWIDTH(7, 9999999.0), "9999999");
  TESTDOUBLE("%.*g", DBLWIDTH(10, 0.123456333), "0.123456333");
  TESTDOUBLE("%g", 123.456222, "123.456");
  TESTDOUBLE("%.*g", DBLWIDTH(10, 123.456222), "123.456222");
  TESTDOUBLE("%g", 600.1234, "600.123");
  TESTDOUBLE("%g", -600.1234, "-600.123");
  TESTDOUBLE("%g", 599.1234, "599.123");
  TESTDOUBLE("%g", -599.1234, "-599.123");

#ifndef _WIN32
  TESTDOUBLE("%g", (double) INFINITY, "inf");
  TESTDOUBLE("%g", (double) -INFINITY, "-inf");
  TESTDOUBLE("%g", (double) NAN, "nan");
#else
  TESTDOUBLE("%g", HUGE_VAL, "inf");
  TESTDOUBLE("%g", -HUGE_VAL, "-inf");
#endif

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_printf(&mjson_print_fixed_buf, &fb, "{%Q:%B}", "a", 1), 10);
    str = "{\"a\":true}";
    ASSERT(memcmp(tmp, str, 10) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    str = "{\"a\":\"\"}";
    ASSERT_EQ(mjson_printf(&mjson_print_fixed_buf, &fb, "{%Q:%Q}", "a", NULL),
              (int) strlen(str));
    ASSERT(memcmp(tmp, str, strlen(str)) == 0);
    ASSERT(fb.len < fb.size);
  }

  {
    char *s = NULL;
    ASSERT_EQ(mjson_printf(&mjson_print_dynamic_buf, &s, "{%Q:%d, %Q:[%s]}", "a",
                           1, "b", "null"), 19);
    ASSERT(s != NULL);
    str = "{\"a\":1, \"b\":[null]}";
    ASSERT(memcmp(s, str, 19) == 0);
    free(s);
  }

  {
    char *s = NULL;
    const char *fmt = "{\"a\":%d, \"b\":%u, \"c\":%ld, \"d\":%lu, \"e\":%M}";
    ASSERT_EQ(mjson_printf(&mjson_print_dynamic_buf, &s, fmt, -1, 3456789012,
                           (long) -1, (unsigned long) 3456789012, f1, 1234), 60);
    ASSERT(s != NULL);
    str =
        "{\"a\":-1, \"b\":3456789012, \"c\":-1, \"d\":3456789012, "
        "\"e\":[1234]}";
    ASSERT(memcmp(s, str, 60) == 0);
    free(s);
  }

  {
    char *s = NULL;
    ASSERT_EQ(mjson_printf(&mjson_print_dynamic_buf, &s, "[%.*Q,%.*s]", 2, "abc",
                           4, "truell"), 11);
    ASSERT(s != NULL);
    str = "[\"ab\",true]";
    ASSERT(memcmp(s, str, 11) == 0);
    free(s);
  }

  {
    char buf[100], *s = mjson_aprintf("[%d]", 123);
    int n = mjson_snprintf(buf, sizeof(buf), "{%g}", 1.23);
    ASSERT(s != NULL);
    ASSERT_EQ(n, 6);
    ASSERT_EQ_STR(s, "[123]");
    ASSERT_EQ_STR(buf, "{1.23}");
    free(s);
  }

  {
    char s[] = "0\n\xfeg";
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_printf(&mjson_print_fixed_buf, &fb, "[%V,%V,%V,%V]", 1, s, 2,
                           s, 3, s, 4, s), 33);
    str = "[\"MA==\",\"MAo=\",\"MAr+\",\"MAr+Zw==\"]";
    ASSERT(memcmp(tmp, str, 33) == 0);
    ASSERT(fb.len < fb.size);
    // printf("%d [%.*s]\n", n, n, tmp);
  }

  {
    int n = mjson_printf(&mjson_print_null, 0, "{%Q:%d}", "a", 1);
    ASSERT_EQ(n, 7);
  }

  {
    char s[] = "\"002001200220616263\"";
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_printf(&mjson_print_fixed_buf, &fb, "%H", 9,
                           "\x00 \x01 \x02 abc"), 20);
    ASSERT_EQ_STR(tmp, s);
  }

  {
    char s[] = "\"a/b\\nc\"";
    struct mjson_fixedbuf fb = {tmp, sizeof(tmp), 0};
    ASSERT_EQ(mjson_printf(&mjson_print_fixed_buf, &fb, "%Q", "a/b\nc"), 8);
    ASSERT_EQ_STR(tmp, s);
  }

  {
    char buf[100];
    ASSERT(mjson_snprintf(buf, sizeof(buf), "[%M, %d, %M, %d]", f1, 123, 42, f1,
                          321, 24) > 0);
    ASSERT_EQ_STR(buf, "[[123], 42, [321], 24]");
  }
}

static void foo(struct jsonrpc_request *r) {
  double v = 0;
  mjson_get_number(r->params, r->params_len, "$[1]", &v);
  jsonrpc_return_success(r, "{%Q:%g,%Q:%Q}", "x", v, "ud", r->userdata);
}

static void foo1(struct jsonrpc_request *r) {
  jsonrpc_return_error(r, 123, "", NULL);
}

static void foo2(struct jsonrpc_request *r) {
  jsonrpc_return_error(r, 456, "qwerty", "%.*s", r->params_len, r->params);
}

static void foo3(struct jsonrpc_request *r) {
  jsonrpc_return_success(r, "%.*s", r->params_len, r->params);
}

static int response_cb(const char *buf, int len, void *privdata) {
  return mjson_printf(mjson_print_fixed_buf, privdata, ">>%.*s<<", len, buf);
}

static void test_rpc(void) {
  // struct jsonrpc_ctx *ctx = &jsonrpc_default_context;
  const char *req = NULL, *res = NULL;
  char buf[200];
  struct mjson_fixedbuf fb = {buf, sizeof(buf), 0};

  // Init context
  jsonrpc_init(response_cb, &fb);

  // Call RPC.List
  req = "{\"id\": 1, \"method\": \"rpc.list\"}";
  res = "{\"id\":1,\"result\":[\"rpc.list\"]}\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Call non-existent method
  req = "{\"id\": 1, \"method\": \"foo\"}\n";
  res =
      "{\"id\":1,\"error\":{\"code\":-32601,\"message\":\"method not "
      "found\"}}\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Register our own function
  req = "{\"id\": 2, \"method\": \"foo\",\"params\":[0,1.23]}\n";
  res = "{\"id\":2,\"result\":{\"x\":1.23,\"ud\":\"hi\"}}\n";
  fb.len = 0;
  jsonrpc_export("foo", foo);
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb,
                  (void *) "hi");
  ASSERT_EQ_STR(buf, res);

  // Test for bad frame
  req = "boo\n";
  res = "{\"error\":{\"code\":-32700,\"message\":\"boo\\n\"}}\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Test simple error response, without data
  req = "{\"id\": 3, \"method\": \"foo1\",\"params\":[1,true]}\n";
  res = "{\"id\":3,\"error\":{\"code\":123,\"message\":\"\"}}\n";
  jsonrpc_export("foo1", foo1);
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Test more complex error response, with data
  req = "{\"id\": 4, \"method\": \"foo2\",\"params\":[1,true]}\n";
  res =
      "{\"id\":4,\"error\":{\"code\":456,\"message\":\"qwerty\",\"data\":[1,"
      "true]}}\n";
  jsonrpc_export("foo2", foo2);
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Test notify - must not generate a response
  req = "{\"method\": \"ping\",\"params\":[1,true]}\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ(fb.len, 0);

  // Test success response
  req = "{\"id\":123,\"result\":[1,2,3]}";
  res = ">>{\"id\":123,\"result\":[1,2,3]}<<";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Test error response
  req = "{\"id\":566,\"error\":{}}";
  res = ">>{\"id\":566,\"error\":{}}<<";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Test glob pattern in the RPC function name
  req = "{\"id\":777,\"method\":\"Bar.Baz\",\"params\":[true]}";
  res = "{\"id\":777,\"result\":[true]}\n";
  jsonrpc_export("Bar.*", foo3);
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);
}

static void test_rpc_batch(void) {
  const char *req, *res;
  char buf[1024];
  struct mjson_fixedbuf fb = {buf, sizeof(buf), 0};

  // Batch with two regular calls
  req = "[{\"id\":1,\"method\":\"foo\",\"params\":[0,1.23]},"
        "{\"id\":2,\"method\":\"foo\",\"params\":[0,4.56]}]";
  res = "[{\"id\":1,\"result\":{\"x\":1.23,\"ud\":\"hi\"}},"
        "{\"id\":2,\"result\":{\"x\":4.56,\"ud\":\"hi\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb,
                  (void *) "hi");
  ASSERT_EQ_STR(buf, res);

  // Batch with a single call
  req = "[{\"id\":10,\"method\":\"foo\",\"params\":[0,9.99]}]";
  res = "[{\"id\":10,\"result\":{\"x\":9.99,\"ud\":\"hi\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb,
                  (void *) "hi");
  ASSERT_EQ_STR(buf, res);

  // Batch with a notification (no id) - should not appear in response
  req = "[{\"id\":1,\"method\":\"foo\",\"params\":[0,1.0]},"
        "{\"method\":\"foo\",\"params\":[0,2.0]},"
        "{\"id\":3,\"method\":\"foo\",\"params\":[0,3.0]}]";
  res = "[{\"id\":1,\"result\":{\"x\":1,\"ud\":\"hi\"}},"
        "{\"id\":3,\"result\":{\"x\":3,\"ud\":\"hi\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb,
                  (void *) "hi");
  ASSERT_EQ_STR(buf, res);

  // Batch with all notifications - no response at all
  req = "[{\"method\":\"foo\",\"params\":[0,1.0]},"
        "{\"method\":\"foo\",\"params\":[0,2.0]}]";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ(fb.len, 0);

  // Empty array - Invalid Request
  req = "[]";
  res = "{\"error\":{\"code\":-32600,\"message\":\"Invalid Request\"}}\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Batch with invalid elements (non-objects)
  req = "[1,2,3]";
  res = "[{\"error\":{\"code\":-32700,\"message\":\"1\"}},"
        "{\"error\":{\"code\":-32700,\"message\":\"2\"}},"
        "{\"error\":{\"code\":-32700,\"message\":\"3\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Batch with mix of valid and invalid elements
  req = "[{\"id\":1,\"method\":\"foo\",\"params\":[0,7.0]},42]";
  res = "[{\"id\":1,\"result\":{\"x\":7,\"ud\":\"hi\"}},"
        "{\"error\":{\"code\":-32700,\"message\":\"42\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb,
                  (void *) "hi");
  ASSERT_EQ_STR(buf, res);

  // Batch with unknown method
  req = "[{\"id\":1,\"method\":\"nonexistent\"}]";
  res = "[{\"id\":1,\"error\":{\"code\":-32601,\"message\":\"method not found\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb, NULL);
  ASSERT_EQ_STR(buf, res);

  // Batch with leading whitespace
  req = "  [{\"id\":1,\"method\":\"foo\",\"params\":[0,5.0]}]";
  res = "[{\"id\":1,\"result\":{\"x\":5,\"ud\":\"hi\"}}]\n";
  fb.len = 0;
  jsonrpc_process(req, (int) strlen(req), mjson_print_fixed_buf, &fb,
                  (void *) "hi");
  ASSERT_EQ_STR(buf, res);
}

static void test_merge(void) {
  char buf[512];
  size_t i;
  const char *tests[] = {
      "",  // Empty string
      "",
      "",
      "{\"a\":1}",  // Simple replace
      "{\"a\":2}",
      "{\"a\":2}",
      "{\"a\":1}",  // Simple add
      "{\"b\":2}",
      "{\"a\":1,\"b\":2}",
      "{\"a\":{}}",  // Object -> scalar
      "{\"a\":1}",
      "{\"a\":1}",
      "{\"a\":{}}",  // Object -> object
      "{\"a\":{\"b\":1}}",
      "{\"a\":{\"b\":1}}",
      "{\"a\":{\"b\":1}}",  // Simple object
      "{\"a\":{\"c\":2}}",
      "{\"a\":{\"b\":1,\"c\":2}}",
      "{\"a\":{\"b\":1,\"c\":2}}",  // Simple object
      "{\"a\":{\"c\":null}}",
      "{\"a\":{\"b\":1}}",
      "{\"a\":[1,{\"b\":false}],\"c\":2}",  // Simple object
      "{\"a\":null,\"b\":[1]}",
      "{\"c\":2,\"b\":[1]}",
      "{\"a\":1}",  // Delete existing key
      "{\"a\":null}",
      "{}",
      "{\"a\":1}",  // Delete non-existing key
      "{\"b\":null}",
      "{\"a\":1}",
  };
  for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i += 3) {
    struct mjson_fixedbuf fb = {buf, sizeof(buf), 0};
    int n = mjson_merge(tests[i], (int) strlen(tests[i]), tests[i + 1],
                        (int) strlen(tests[i + 1]), mjson_print_fixed_buf, &fb);
    // printf("%s + %s = %.*s\n", tests[i], tests[i + 1], fb.len, fb.ptr);
    ASSERT_EQ(n, (int) strlen(tests[i + 2]));
    ASSERT(strncmp(fb.ptr, tests[i + 2], (size_t) fb.len) == 0);
  }
}

static void test_pretty(void) {
  size_t i;
  const char *tests[] = {
      "{   }",  // Empty object
      "{}",
      "{}",
      "[   ]",  // Empty array
      "[]",
      "[]",
      "{ \"a\" :1    }",  // Simple object
      "{\n  \"a\": 1\n}",
      "{\"a\":1}",
      "{ \"a\" :1  ,\"b\":2}",  // Simple object, 2 keys
      "{\n  \"a\": 1,\n  \"b\": 2\n}",
      "{\"a\":1,\"b\":2}",
      "{ \"a\" :1  ,\"b\":2, \"c\":[1,2,{\"d\":3}]}",  // Complex object
      "{\n  \"a\": 1,\n  \"b\":"
      " 2,\n  \"c\": [\n    1,\n    2,\n  "
      "  {\n      \"d\": 3\n    }\n  ]\n}",
      "{\"a\":1,\"b\":2,\"c\":[1,2,{\"d\":3}]}",

      "{ \"a\" :{\"b\"  :2},\"c\": {}    }",  // Nested object
      "{\n  \"a\": {\n    \"b\": 2\n  },\n  \"c\": {}\n}",
      "{\"a\":{\"b\":2},\"c\":{}}",
  };

  for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i += 3) {
    char buf[512];
    struct mjson_fixedbuf fb = {buf, sizeof(buf), 0};
    const char *s = tests[i];
    ASSERT(mjson_pretty(s, (int) strlen(s), "  ", mjson_print_fixed_buf, &fb) >
           0);
    ASSERT_EQ(fb.len, (int) strlen(tests[i + 1]));
    ASSERT(strncmp(fb.ptr, tests[i + 1], (size_t) fb.len) == 0);
    // printf("==> %s\n", buf);

    // Terse print
    fb.len = 0;
    ASSERT(mjson_pretty(s, (int) strlen(s), "", mjson_print_fixed_buf, &fb) >
           0);
    ASSERT_EQ(fb.len, (int) strlen(tests[i + 2]));
    ASSERT(strncmp(fb.ptr, tests[i + 2], (size_t) fb.len) == 0);
    // printf("--> %s\n", buf);
  }
}

static void test_next(void) {
  int a, b, c, d, t;

  {
    const char *s = "{}";
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 0, &a, &b, &c, &d, &t), 0);
  }

  {
    const char *s = "{\"a\":1}";
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 0, &a, &b, &c, &d, &t), 6);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 5);
    ASSERT_EQ(d, 1);
    ASSERT_EQ(t, MJSON_TOK_NUMBER);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 6, &a, &b, &c, &d, &t), 0);
  }

  {
    const char *s = "{\"a\":123,\"b\":[1,2,3,{\"c\":1}],\"d\":null}";
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 0, &a, &b, &c, &d, &t), 8);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 5);
    ASSERT_EQ(d, 3);
    ASSERT_EQ(t, MJSON_TOK_NUMBER);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 8, &a, &b, &c, &d, &t), 28);
    ASSERT_EQ(a, 9);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 13);
    ASSERT_EQ(d, 15);
    ASSERT_EQ(t, MJSON_TOK_ARRAY);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 28, &a, &b, &c, &d, &t), 37);
    ASSERT_EQ(a, 29);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 33);
    ASSERT_EQ(d, 4);
    ASSERT_EQ(t, MJSON_TOK_NULL);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 37, &a, &b, &c, &d, &t), 0);
  }

  {
    const char *s = "[]";
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 0, &a, &b, &c, &d, &t), 0);
  }

  {
    const char *s = "[3,null,{},[1,2],{\"x\":[3]},\"hi\"]";
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 0, &a, &b, &c, &d, &t), 2);
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 1);
    ASSERT_EQ(d, 1);
    ASSERT_EQ(t, MJSON_TOK_NUMBER);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 2, &a, &b, &c, &d, &t), 7);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 3);
    ASSERT_EQ(d, 4);
    ASSERT_EQ(t, MJSON_TOK_NULL);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 7, &a, &b, &c, &d, &t), 10);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 8);
    ASSERT_EQ(d, 2);
    ASSERT_EQ(t, MJSON_TOK_OBJECT);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 10, &a, &b, &c, &d, &t), 16);
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 11);
    ASSERT_EQ(d, 5);
    ASSERT_EQ(t, MJSON_TOK_ARRAY);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 16, &a, &b, &c, &d, &t), 26);
    ASSERT_EQ(a, 4);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 17);
    ASSERT_EQ(d, 9);
    ASSERT_EQ(t, MJSON_TOK_OBJECT);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 26, &a, &b, &c, &d, &t), 31);
    ASSERT_EQ(a, 5);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 27);
    ASSERT_EQ(d, 4);
    ASSERT_EQ(t, MJSON_TOK_STRING);
    ASSERT_EQ(mjson_next(s, (int) strlen(s), 31, &a, &b, &c, &d, &t), 0);
  }
}

static void test_globmatch(void) {
  ASSERT_EQ(mjson_globmatch("", 0, "", 0), 1);
  ASSERT_EQ(mjson_globmatch("*", 1, "a", 1), 1);
  ASSERT_EQ(mjson_globmatch("*", 1, "ab", 2), 1);
  ASSERT_EQ(mjson_globmatch("", 0, "a", 1), 0);
  ASSERT_EQ(mjson_globmatch("/", 1, "/foo", 4), 0);
  ASSERT_EQ(mjson_globmatch("/*/foo", 6, "/x/bar", 6), 0);
  ASSERT_EQ(mjson_globmatch("/*/foo", 6, "/x/foo", 6), 1);
  ASSERT_EQ(mjson_globmatch("/*/foo", 6, "/x/foox", 7), 0);
  ASSERT_EQ(mjson_globmatch("/*/foo*", 7, "/x/foox", 7), 1);
  ASSERT_EQ(mjson_globmatch("/*", 2, "/abc", 4), 1);
  ASSERT_EQ(mjson_globmatch("/*", 2, "/ab/", 4), 0);
  ASSERT_EQ(mjson_globmatch("/*", 2, "/", 1), 1);
  ASSERT_EQ(mjson_globmatch("/x/*", 4, "/x/2", 4), 1);
  ASSERT_EQ(mjson_globmatch("/x/*", 4, "/x/2/foo", 8), 0);
  ASSERT_EQ(mjson_globmatch("/x/*/*", 6, "/x/2/foo", 8), 1);
  ASSERT_EQ(mjson_globmatch("#", 1, "///", 3), 1);
}

static void test_multiple_contexts(void) {
  struct jsonrpc_ctx c1, c2;
  const char *req = "{\"id\": 1, \"method\": \"rpc.list\"}";
  const char *exp1 = "{\"id\":1,\"result\":[\"rpc.list\",\"foo\"]}\n";
  const char *exp2 = "{\"id\":1,\"result\":[\"rpc.list\"]}\n";
  int rl = (int) strlen(req);
  char *r1 = NULL, *r2 = NULL;

  jsonrpc_ctx_init(&c1, NULL, NULL);
  jsonrpc_ctx_init(&c2, NULL, NULL);

  jsonrpc_ctx_export(&c1, "foo", foo);
  jsonrpc_ctx_export(&c1, MJSON_RPC_LIST_NAME, jsonrpc_list);
  jsonrpc_ctx_export(&c2, MJSON_RPC_LIST_NAME, jsonrpc_list);

  jsonrpc_ctx_process(&c1, req, rl, mjson_print_dynamic_buf, &r1, NULL);
  jsonrpc_ctx_process(&c2, req, rl, mjson_print_dynamic_buf, &r2, NULL);
  ASSERT_EQ_STR(r1, exp1);
  ASSERT_EQ_STR(r2, exp2);
  free(r1);
  free(r2);
}

int main() {
  printf("mjson Test Suite         Total Fails\r\n");
  printf("--------------------------------------------\r\n");

  RUN_SUITE(test_multiple_contexts);
  RUN_SUITE(test_next);
  RUN_SUITE(test_printf);
  RUN_SUITE(test_cb);
  RUN_SUITE(test_find);
  RUN_SUITE(test_get_number);
  RUN_SUITE(test_get_bool);
  RUN_SUITE(test_get_string);
  RUN_SUITE(test_print);
  RUN_SUITE(test_rpc);
  RUN_SUITE(test_rpc_batch);
  RUN_SUITE(test_merge);
  RUN_SUITE(test_pretty);
  RUN_SUITE(test_globmatch);

  printf("--------------------------------------------\r\n");
  printf("%-24s  %4d  %4d\r\n", s_num_errors ? "FAILURE" : "SUCCESS", s_num_tests, s_num_errors);
  printf("--------------------------------------------\r\n");
  printf("mjson Test Suite         Total Fails\r\n");
  return s_num_errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
