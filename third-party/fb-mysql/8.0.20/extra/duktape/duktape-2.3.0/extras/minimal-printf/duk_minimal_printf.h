#if !defined(DUK_MINIMAL_PRINTF_H_INCLUDED)
#define DUK_MINIMAL_PRINTF_H_INCLUDED

#include <stdarg.h>  /* va_list etc */
#include <stddef.h>  /* size_t */

#if defined(__cplusplus)
extern "C" {
#endif

extern int duk_minimal_sprintf(char *str, const char *format, ...);
extern int duk_minimal_snprintf(char *str, size_t size, const char *format, ...);
extern int duk_minimal_vsnprintf(char *str, size_t size, const char *format, va_list ap);
extern int duk_minimal_sscanf(const char *str, const char *format, ...);

#if defined(__cplusplus)
}
#endif  /* end 'extern "C"' wrapper */

#endif  /* DUK_MINIMAL_PRINTF_H_INCLUDED */
