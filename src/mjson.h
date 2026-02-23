// Copyright (c) 2018-2020 Cesanta Software Limited
// All rights reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MJSON_H
#define MJSON_H

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/**
 * @defgroup mjson_config Configuration Macros
 * @brief Compile-time configuration options for mJSON library.
 * @{
 */

/**
 * @brief Enable/disable printing functionality.
 * @details Set to 0 to disable all print-related functions and reduce code size.
 */
#ifndef MJSON_ENABLE_PRINT
#define MJSON_ENABLE_PRINT 1
#endif

/**
 * @brief Enable/disable RPC (Remote Procedure Call) functionality.
 * @details Set to 0 to disable JSON-RPC support and reduce code size.
 */
#ifndef MJSON_ENABLE_RPC
#define MJSON_ENABLE_RPC 1
#endif

/**
 * @brief Enable/disable Base64 encoding/decoding functionality.
 * @details Set to 0 to disable Base64 support and reduce code size.
 */
#ifndef MJSON_ENABLE_BASE64
#define MJSON_ENABLE_BASE64 1
#endif

/**
 * @brief Enable/disable JSON merging functionality.
 * @details Set to 0 to disable JSON merge support and reduce code size.
 */
#ifndef MJSON_ENABLE_MERGE
#define MJSON_ENABLE_MERGE 1
#endif

/**
 * @brief Enable/disable pretty-printing functionality.
 * @details Set to 0 to disable pretty-print support and reduce code size.
 */
#ifndef MJSON_ENABLE_PRETTY
#define MJSON_ENABLE_PRETTY 1
#endif

/**
 * @brief Enable/disable mjson_next() functionality.
 * @details Set to 0 to disable iterator support and reduce code size.
 */
#ifndef MJSON_ENABLE_NEXT
#define MJSON_ENABLE_NEXT 1
#endif

/**
 * @brief Method name for listing available RPC methods.
 * @details Used by jsonrpc_list() to expose available methods via RPC.
 */
#ifndef MJSON_RPC_LIST_NAME
#define MJSON_RPC_LIST_NAME "rpc.list"
#endif

/**
 * @brief Allocation granularity for dynamic print buffer.
 * @details Specifies the chunk size in bytes for print_dynamic_buf() reallocations.
 */
#ifndef MJSON_DYNBUF_CHUNK
#define MJSON_DYNBUF_CHUNK 256
#endif

/**
 * @brief Custom memory reallocation function.
 * @details Override this macro to use a custom memory allocator.
 */
#ifndef MJSON_REALLOC
#define MJSON_REALLOC realloc
#endif

/** @} */ // end of mjson_config

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup mjson_errors Error Codes
 * @brief Error return codes for mJSON functions.
 * @{
 */

/** @brief Invalid input error code. */
#define MJSON_ERROR_INVALID_INPUT (-1)

/** @brief Maximum nesting depth exceeded error code. */
#define MJSON_ERROR_TOO_DEEP (-2)

/** @} */ // end of mjson_errors

/**
 * @defgroup mjson_tokens Token Types
 * @brief JSON token type identifiers returned by the parser.
 * @{
 */

/** @brief Invalid token. */
#define MJSON_TOK_INVALID   0

/** @brief JSON key token (object member name). */
#define MJSON_TOK_KEY       1

/** @brief JSON string token. */
#define MJSON_TOK_STRING    11

/** @brief JSON number token. */
#define MJSON_TOK_NUMBER    12

/** @brief JSON boolean true token. */
#define MJSON_TOK_TRUE      13

/** @brief JSON boolean false token. */
#define MJSON_TOK_FALSE     14

/** @brief JSON null token. */
#define MJSON_TOK_NULL      15

/** @brief JSON array token (character code for '['). */
#define MJSON_TOK_ARRAY     91

/** @brief JSON object token (character code for '{'). */
#define MJSON_TOK_OBJECT    123

/**
 * @brief Check if token type represents a JSON value.
 * @details Returns true if the token is a primitive value type (string, number, true, false, null).
 * @param t Token type to check.
 * @return Non-zero if token is a value type, zero otherwise.
 */
#define MJSON_TOK_IS_VALUE(t) ((t) > 10 && (t) < 20)

/** @} */ // end of mjson_tokens

/**
 * @defgroup mjson_typedefs Type Definitions
 * @brief Callback and function pointer type definitions.
 * @{
 */

/**
 * @brief Callback function type for mjson() parser.
 * @param event Event type (one of MJSON_TOK_* values).
 * @param buf Pointer to the JSON buffer being parsed.
 * @param offset Offset in the buffer where the token starts.
 * @param len Length of the token in bytes.
 * @param fn_data User-defined data pointer passed to mjson().
 * @return 0 to continue parsing, non-zero to stop.
 */
typedef int (*mjson_cb_t)(int event, const char *buf, int offset, int len,
                          void *fn_data);

/**
 * @brief Maximum nesting depth for JSON parser.
 * @details Controls the maximum depth of nested objects/arrays.
 */
#ifndef MJSON_MAX_DEPTH
#define MJSON_MAX_DEPTH 20
#endif

/** @} */ // end of mjson_typedefs

/**
 * @defgroup mjson_core Core JSON Functions
 * @brief Core JSON parsing and querying functions.
 * @{
 */

/**
 * @brief Parse a JSON string and invoke callback for each token.
 * @param buf Pointer to the JSON string to parse.
 * @param len Length of the JSON string in bytes.
 * @param cb Callback function to invoke for each token.
 * @param ud User-defined data pointer passed to the callback.
 * @return 0 on success, negative error code on failure.
 */
int mjson(const char *buf, int len, mjson_cb_t cb, void *ud);

/**
 * @brief Find a value in JSON by JSON path.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param jp JSON path expression (e.g., "$.foo.bar" or "$[0]").
 * @param tp Pointer to store the token type of the found value.
 * @param tl Pointer to store the token length of the found value.
 * @return Offset of the found value in the buffer, or -1 if not found.
 */
int mjson_find(const char *buf, int len, const char *jp, const char **tp,
               int *tl);

/**
 * @brief Extract a number value from JSON by path.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param path JSON path expression.
 * @param v Pointer to store the extracted number value.
 * @return 1 on success, 0 if not found or invalid type.
 */
int mjson_get_number(const char *buf, int len, const char *path, double *v);

/**
 * @brief Extract a boolean value from JSON by path.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param path JSON path expression.
 * @param v Pointer to store the extracted boolean value (0 or 1).
 * @return 1 on success, 0 if not found or invalid type.
 */
int mjson_get_bool(const char *buf, int len, const char *path, int *v);

/**
 * @brief Extract a string value from JSON by path.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param path JSON path expression.
 * @param to Destination buffer to store the extracted string.
 * @param n Size of the destination buffer.
 * @return Length of the extracted string, or -1 on error.
 */
int mjson_get_string(const char *buf, int len, const char *path, char *to,
                     int n);

/**
 * @brief Extract a hex-encoded string from JSON by path and decode it.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param path JSON path expression.
 * @param to Destination buffer to store the decoded data.
 * @param n Size of the destination buffer.
 * @return Length of the decoded data, or -1 on error.
 */
int mjson_get_hex(const char *buf, int len, const char *path, char *to, int n);

/** @} */ // end of mjson_core

#if MJSON_ENABLE_NEXT
/**
 * @defgroup mjson_iterator Iterator Functions
 * @brief Functions for iterating over JSON objects and arrays.
 * @{
 */

/**
 * @brief Iterate to the next element in a JSON object or array.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param offset Current offset in the buffer (use 0 to start iteration).
 * @param key_offset Pointer to store the offset of the key (for objects).
 * @param key_len Pointer to store the length of the key (for objects).
 * @param val_offset Pointer to store the offset of the value.
 * @param val_len Pointer to store the length of the value.
 * @param val_type Pointer to store the type of the value.
 * @return Offset of the next element, or 0 when iteration is complete.
 */
int mjson_next(const char *buf, int len, int offset, int *key_offset,
               int *key_len, int *val_offset, int *val_len, int *val_type);

/** @} */ // end of mjson_iterator
#endif

#if MJSON_ENABLE_BASE64
/**
 * @defgroup mjson_base64 Base64 Functions
 * @brief Base64 encoding and decoding utilities.
 * @{
 */

/**
 * @brief Extract a Base64-encoded string from JSON by path and decode it.
 * @param buf Pointer to the JSON string.
 * @param len Length of the JSON string in bytes.
 * @param path JSON path expression.
 * @param dst Destination buffer to store the decoded data.
 * @param dst_len Size of the destination buffer.
 * @return Length of the decoded data, or -1 on error.
 */
int mjson_get_base64(const char *buf, int len, const char *path, char *dst,
                     int dst_len);

/**
 * @brief Decode a Base64-encoded string.
 * @param src Pointer to the Base64-encoded input string.
 * @param src_len Length of the input string in bytes.
 * @param dst Destination buffer to store the decoded data.
 * @param dst_len Size of the destination buffer.
 * @return Length of the decoded data, or -1 on error.
 */
int mjson_base64_dec(const char *src, int src_len, char *dst, int dst_len);

/** @} */ // end of mjson_base64
#endif

#if MJSON_ENABLE_PRINT
/**
 * @defgroup mjson_print Print Functions
 * @brief JSON printing and formatting utilities.
 * @{
 */

/**
 * @brief Print callback function type.
 * @param buf Pointer to the buffer to print.
 * @param len Length of the buffer in bytes.
 * @param fn_data User-defined data pointer.
 * @return Number of bytes printed, or negative on error.
 */
typedef int (*mjson_print_fn_t)(const char *buf, int len, void *fn_data);

/**
 * @brief Variable-argument print function type.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param ap Variable argument list.
 * @return Number of bytes printed, or negative on error.
 */
typedef int (*mjson_vprint_fn_t)(mjson_print_fn_t fn, void *, va_list *);

/**
 * @brief Fixed-size buffer structure for printing.
 */
struct mjson_fixedbuf {
  char *ptr;      /**< Pointer to the buffer. */
  int size;       /**< Total size of the buffer. */
  int len;        /**< Current length of data in the buffer. */
};

/**
 * @brief Print formatted output using printf-style format string.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param fmt Format string (printf-style).
 * @param ... Variable arguments for the format string.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_printf(mjson_print_fn_t fn, void *fn_data, const char *fmt, ...);

/**
 * @brief Print formatted output using variable argument list.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param fmt Format string (printf-style).
 * @param ap Variable argument list.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_vprintf(mjson_print_fn_t fn, void *fn_data, const char *fmt,
                  va_list *ap);

/**
 * @brief Print a string value (with JSON escaping).
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param buf Pointer to the string to print.
 * @param len Length of the string in bytes.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_str(mjson_print_fn_t fn, void *fn_data, const char *buf,
                    int len);

/**
 * @brief Print an integer value.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param value Integer value to print.
 * @param is_signed Non-zero if the value is signed.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_int(mjson_print_fn_t fn, void *fn_data, int value,
                    int is_signed);

/**
 * @brief Print a long integer value.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param value Long integer value to print.
 * @param is_signed Non-zero if the value is signed.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_long(mjson_print_fn_t fn, void *fn_data, long value,
                     int is_signed);

/**
 * @brief Print a raw buffer (no escaping).
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param buf Pointer to the buffer to print.
 * @param len Length of the buffer in bytes.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_buf(mjson_print_fn_t fn, void *fn_data, const char *buf,
                    int len);

/**
 * @brief Print a double-precision floating-point value.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @param d Double value to print.
 * @param width Minimum field width (use 0 for default).
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_dbl(mjson_print_fn_t fn, void *fn_data, double d, int width);

/**
 * @brief Print function that discards output (for measuring length).
 * @param ptr Pointer to the buffer (ignored).
 * @param len Length of the buffer in bytes.
 * @param fn_data User-defined data pointer (ignored).
 * @return Always returns len.
 */
int mjson_print_null(const char *ptr, int len, void *fn_data);

/**
 * @brief Print function that writes to a fixed-size buffer.
 * @param ptr Pointer to the buffer to print.
 * @param len Length of the buffer in bytes.
 * @param fn_data Pointer to struct mjson_fixedbuf.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_fixed_buf(const char *ptr, int len, void *fn_data);

/**
 * @brief Print function that writes to a dynamically growing buffer.
 * @param ptr Pointer to the buffer to print.
 * @param len Length of the buffer in bytes.
 * @param fn_data Pointer to struct mjson_fixedbuf (will be reallocated as needed).
 * @return Number of bytes printed, or negative on error.
 */
int mjson_print_dynamic_buf(const char *ptr, int len, void *fn_data);

/**
 * @brief Format output into a fixed-size buffer (snprintf-style).
 * @param buf Destination buffer.
 * @param len Size of the destination buffer.
 * @param fmt Format string (printf-style).
 * @param ... Variable arguments for the format string.
 * @return Number of characters that would have been written (excluding null terminator).
 */
int mjson_snprintf(char *buf, size_t len, const char *fmt, ...);

/**
 * @brief Format output into a dynamically allocated string.
 * @param fmt Format string (printf-style).
 * @param ... Variable arguments for the format string.
 * @return Pointer to allocated string (must be freed with free()), or NULL on error.
 */
char *mjson_aprintf(const char *fmt, ...);

#if MJSON_ENABLE_PRETTY
/**
 * @brief Pretty-print JSON with indentation.
 * @param s Pointer to the JSON string.
 * @param n Length of the JSON string in bytes.
 * @param pad String to use for each indentation level (e.g., "  " for 2 spaces).
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_pretty(const char *s, int n, const char *pad, mjson_print_fn_t fn,
                 void *fn_data);
#endif

#if MJSON_ENABLE_MERGE
/**
 * @brief Merge two JSON objects.
 * @details Values from s2 override values from s for matching keys.
 * @param s Pointer to the first JSON string (base object).
 * @param n Length of the first JSON string in bytes.
 * @param s2 Pointer to the second JSON string (override object).
 * @param n2 Length of the second JSON string in bytes.
 * @param fn Print callback function.
 * @param fn_data User-defined data pointer.
 * @return Number of bytes printed, or negative on error.
 */
int mjson_merge(const char *s, int n, const char *s2, int n2,
                mjson_print_fn_t fn, void *fn_data);
#endif

/** @} */ // end of mjson_print
#endif  // MJSON_ENABLE_PRINT

#if MJSON_ENABLE_RPC
/**
 * @defgroup mjson_rpc JSON-RPC Functions
 * @brief JSON-RPC 2.0 implementation utilities.
 * @{
 */

/**
 * @brief Initialize the default JSON-RPC context.
 * @param response_cb Callback function for sending responses.
 * @param fn_data User-defined data pointer for the callback.
 */
void jsonrpc_init(mjson_print_fn_t response_cb, void *fn_data);

/**
 * @brief Match a string against a glob pattern.
 * @param s1 Pointer to the first string.
 * @param n1 Length of the first string in bytes.
 * @param s2 Pointer to the pattern string.
 * @param n2 Length of the pattern string in bytes.
 * @return Non-zero if strings match, zero otherwise.
 */
int mjson_globmatch(const char *s1, int n1, const char *s2, int n2);

/**
 * @brief JSON-RPC request structure.
 * @details Contains all information about a parsed JSON-RPC request.
 */
struct jsonrpc_request {
  struct jsonrpc_ctx *ctx;    /**< Pointer to the RPC context. */
  const char *frame;          /**< Pointer to the whole JSON-RPC frame. */
  int frame_len;              /**< Length of the frame in bytes. */
  const char *params;         /**< Pointer to the "params" value. */
  int params_len;             /**< Length of the params in bytes. */
  const char *id;             /**< Pointer to the "id" value. */
  int id_len;                 /**< Length of the id in bytes. */
  const char *method;         /**< Pointer to the "method" name. */
  int method_len;             /**< Length of the method name in bytes. */
  mjson_print_fn_t fn;        /**< Printer function for response. */
  void *fn_data;              /**< Printer function data. */
  void *userdata;             /**< User data specified at export time. */
};

/**
 * @brief JSON-RPC method registration structure.
 */
struct jsonrpc_method {
  const char *method;                 /**< Method name. */
  int method_sz;                      /**< Length of the method name. */
  void (*cb)(struct jsonrpc_request *); /**< Callback function for the method. */
  struct jsonrpc_method *next;        /**< Pointer to next method in list. */
};

/**
 * @brief JSON-RPC context structure.
 * @details Stores current request information and registered RPC methods.
 */
struct jsonrpc_ctx {
  struct jsonrpc_method *methods;     /**< Linked list of registered methods. */
  mjson_print_fn_t response_cb;       /**< Response callback function. */
  void *response_cb_data;             /**< Response callback user data. */
};

/**
 * @brief Register an RPC method within a specific context.
 * @param ctx Pointer to the JSON-RPC context.
 * @param name Name of the RPC method to register.
 * @param fn Callback function to invoke when the method is called.
 */
#define jsonrpc_ctx_export(ctx, name, fn)                                 \
  do {                                                                    \
    static struct jsonrpc_method m = {(name), sizeof(name) - 1, (fn), 0}; \
    m.next = (ctx)->methods;                                              \
    (ctx)->methods = &m;                                                  \
  } while (0)

/**
 * @brief Initialize a JSON-RPC context.
 * @param ctx Pointer to the context structure to initialize.
 * @param response_cb Callback function for sending responses.
 * @param response_cb_data User-defined data pointer for the callback.
 */
void jsonrpc_ctx_init(struct jsonrpc_ctx *ctx, mjson_print_fn_t response_cb,
                      void *response_cb_data);

/**
 * @brief Send a JSON-RPC error response.
 * @param r Pointer to the request structure.
 * @param code Error code (use JSONRPC_ERROR_* constants).
 * @param message Human-readable error message.
 * @param data_fmt Optional format string for additional error data (printf-style).
 * @param ... Variable arguments for the data format string.
 */
void jsonrpc_return_error(struct jsonrpc_request *r, int code,
                          const char *message, const char *data_fmt, ...);

/**
 * @brief Send a JSON-RPC success response.
 * @param r Pointer to the request structure.
 * @param result_fmt Format string for the result value (printf-style).
 * @param ... Variable arguments for the result format string.
 */
void jsonrpc_return_success(struct jsonrpc_request *r, const char *result_fmt,
                            ...);

/**
 * @brief Process a JSON-RPC request string.
 * @param ctx Pointer to the JSON-RPC context.
 * @param req Pointer to the JSON-RPC request string.
 * @param req_sz Length of the request string in bytes.
 * @param fn Print callback function for the response.
 * @param fn_data User-defined data pointer for the callback.
 * @param userdata User-defined data passed to the method callback.
 */
void jsonrpc_ctx_process(struct jsonrpc_ctx *ctx, const char *req, int req_sz,
                         mjson_print_fn_t fn, void *fn_data, void *userdata);

/**
 * @brief Default JSON-RPC context for global operations.
 */
extern struct jsonrpc_ctx jsonrpc_default_context;

/**
 * @brief Built-in RPC method that lists all registered methods.
 * @param r Pointer to the request structure.
 */
extern void jsonrpc_list(struct jsonrpc_request *r);

/**
 * @brief Register an RPC method in the default context.
 * @param name Name of the RPC method to register.
 * @param fn Callback function to invoke when the method is called.
 */
#define jsonrpc_export(name, fn) \
  jsonrpc_ctx_export(&jsonrpc_default_context, (name), (fn))

/**
 * @brief Process a JSON-RPC request using the default context.
 * @param buf Pointer to the JSON-RPC request string.
 * @param len Length of the request string in bytes.
 * @param fn Print callback function for the response.
 * @param fnd User-defined data pointer for the callback.
 * @param ud User-defined data passed to the method callback.
 */
#define jsonrpc_process(buf, len, fn, fnd, ud) \
  jsonrpc_ctx_process(&jsonrpc_default_context, (buf), (len), (fn), (fnd), (ud))

/**
 * @defgroup mjson_rpc_errors JSON-RPC Error Codes
 * @brief Standard JSON-RPC 2.0 error codes.
 * @{
 */

/** @brief Invalid JSON was received by the server (-32700). */
#define JSONRPC_ERROR_INVALID     -32700

/** @brief The method does not exist (-32601). */
#define JSONRPC_ERROR_NOT_FOUND   -32601

/** @brief Invalid method parameters (-32602). */
#define JSONRPC_ERROR_BAD_PARAMS  -32602

/** @brief Internal JSON-RPC error (-32603). */
#define JSONRPC_ERROR_INTERNAL    -32603

/** @} */ // end of mjson_rpc_errors

/** @} */ // end of mjson_rpc
#endif  // MJSON_ENABLE_RPC

#ifdef __cplusplus
}
#endif
#endif  // MJSON_H