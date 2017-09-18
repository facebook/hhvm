/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2017 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_H
#define ZEND_H

#ifdef __cplusplus
#  define BEGIN_EXTERN_C() extern "C" {
#  define END_EXTERN_C() }
#else
#  define BEGIN_EXTERN_C()
#  define END_EXTERN_C()
#endif // __cplusplus

#ifdef __cplusplus
#include <cinttypes>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cerrno>
#include <cstdarg>
#include <climits>
#include <stdexcept>
#else
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#endif // __cplusplus

#if defined(__cplusplus)
#define NORETURN [[noreturn]]

struct ParseException : public std::logic_error {
  explicit ParseException(const std::string& what)
    : std::logic_error(what) {}
};

#else
#define NORETURN __attribute__((noreturn))
#endif

BEGIN_EXTERN_C()

#define ZEND_PORTABILITY_H // to prevent its normal inclusion

#define ZEND_API
//#define ZEND_TLS _Thread_local
#define ZEND_ASSERT(_)
#define zend_always_inline inline

void* emalloc(size_t);
void* ecalloc(size_t, size_t);
void* erealloc(void*, size_t);
void* safe_erealloc(void*, size_t num, size_t size, size_t offset);
void efree(void*);
char* estrdup(const char*);
char* estrndup(const char*, size_t);

#define pemalloc(size, _) emalloc(size)
#define perealloc(ptr, size, _) erealloc(ptr, size)
#define pefree(ptr, _) efree(ptr)

#define ZEND_MM_ALIGNED_SIZE(n) (n)

// XXX: could replace these with the ones from folly
#define EXPECTED(c) (c)
#define UNEXPECTED(c) (c)

////////////////////////////////////////////////////////////////////////////////
// from zend.h

NORETURN void zend_error_noreturn(int type, const char*fmt, ...);
void zend_error(int type, const char*fmt, ...);

#define SIZEOF_SIZE_T 8

#include "zend_types.h"
#include "zend_vm_opcodes.h"
#include "zend_errors.h"
#include "zend_multiply.h"
#include "zend_arena.h"
#include "zend_stack.h"
#include "zend_ptr_stack.h"
#include "zend_strtod.h"
#include "zend_ast.h"
#include "zend_language_parser.h"

#define ZSTR_VAL(zs) (zs)->val
#define ZSTR_LEN(zs) (zs)->len
#define ZSTR_EMPTY_ALLOC() zend_string_init("", 0, 0)


void zend_string_release(zend_string*);
zend_string* zend_string_init(const char*, size_t, int);
zend_string* zend_string_extend(zend_string*, size_t, int);

#define ZEND_STR_STATIC "static"

// hacks ahead
#define ZSTR_KNOWN(str) zend_string_init(str, strlen(str), 0)

////////////////////////////////////////////////////////////////////////////////
// zend_API.h

#define ZVAL_STRINGL(z, s, l) do {				\
		ZVAL_NEW_STR(z, zend_string_init(s, l, 0));		\
	} while (0)

#define ZVAL_STRING(z, s) do {					\
		const char *_s = (s);					\
		ZVAL_STRINGL(z, _s, strlen(_s));		\
	} while (0)


////////////////////////////////////////////////////////////////////////////////
// adapted from zend_compile.h

// what on *earth* this was doing in zend_compile nobody knows
typedef union _zend_parser_stack_elem {
	zend_ast *ast;
	zend_string *str;
	zend_ulong num;
} zend_parser_stack_elem;

zend_ast* zend_ast_append_str(zend_ast*, zend_ast*);
zend_ast *zend_negate_num_string(zend_ast *ast);

#define RESET_DOC_COMMENT() do { \
	if (CG(doc_comment)) { \
		zend_string_release(CG(doc_comment)); \
		CG(doc_comment) = NULL; \
	} \
} while (0)

/* class fetches */
#define ZEND_FETCH_CLASS_DEFAULT	0
#define ZEND_FETCH_CLASS_SELF		1
#define ZEND_FETCH_CLASS_PARENT		2
#define ZEND_FETCH_CLASS_STATIC		3
#define ZEND_FETCH_CLASS_AUTO		4
#define ZEND_FETCH_CLASS_INTERFACE	5
#define ZEND_FETCH_CLASS_TRAIT		6
#define ZEND_FETCH_CLASS_MASK        0x0f
#define ZEND_FETCH_CLASS_NO_AUTOLOAD 0x80
#define ZEND_FETCH_CLASS_SILENT      0x0100
#define ZEND_FETCH_CLASS_EXCEPTION   0x0200

#define ZEND_PARAM_REF      (1<<0)
#define ZEND_PARAM_VARIADIC (1<<1)

#define ZEND_NAME_FQ       0
#define ZEND_NAME_NOT_FQ   1
#define ZEND_NAME_RELATIVE 2

#define ZEND_TYPE_NULLABLE (1<<8)

#define ZEND_ARRAY_SYNTAX_LIST 1  /* list() */
#define ZEND_ARRAY_SYNTAX_LONG 2  /* array() */
#define ZEND_ARRAY_SYNTAX_SHORT 3 /* [] */

/* var status for backpatching */
#define BP_VAR_R			0
#define BP_VAR_W			1
#define BP_VAR_RW			2
#define BP_VAR_IS			3
#define BP_VAR_FUNC_ARG		4
#define BP_VAR_UNSET		5

/* Bottom 3 bits are the type, top bits are arg num for BP_VAR_FUNC_ARG */
#define BP_VAR_SHIFT 3
#define BP_VAR_MASK  7


#define ZEND_INTERNAL_FUNCTION				1
#define ZEND_USER_FUNCTION					2
#define ZEND_OVERLOADED_FUNCTION			3
#define	ZEND_EVAL_CODE						4
#define ZEND_OVERLOADED_FUNCTION_TEMPORARY	5

/* A quick check (type == ZEND_USER_FUNCTION || type == ZEND_EVAL_CODE) */
#define ZEND_USER_CODE(type) ((type & 1) == 0)

#define ZEND_INTERNAL_CLASS         1
#define ZEND_USER_CLASS             2

#define ZEND_EVAL				(1<<0)
#define ZEND_INCLUDE			(1<<1)
#define ZEND_INCLUDE_ONCE		(1<<2)
#define ZEND_REQUIRE			(1<<3)
#define ZEND_REQUIRE_ONCE		(1<<4)

#define ZEND_CT	(1<<0)
#define ZEND_RT (1<<1)

/* global/local fetches */
#define ZEND_FETCH_GLOBAL			0x00000000
#define ZEND_FETCH_LOCAL			0x10000000
#define ZEND_FETCH_GLOBAL_LOCK		0x40000000

#define ZEND_FETCH_TYPE_MASK		0x70000000

#define ZEND_FETCH_STANDARD		    0x00000000

#define ZEND_ISSET				    0x02000000
#define ZEND_ISEMPTY			    0x01000000
#define ZEND_ISSET_ISEMPTY_MASK	    (ZEND_ISSET | ZEND_ISEMPTY)
#define ZEND_QUICK_SET			    0x00800000

#define ZEND_FETCH_ARG_MASK         0x000fffff

#define ZEND_FREE_ON_RETURN     (1<<0)

#define ZEND_SEND_BY_VAL     0
#define ZEND_SEND_BY_REF     1
#define ZEND_SEND_PREFER_REF 2

#define ZEND_DIM_IS 1

/* method flags (types) */
#define ZEND_ACC_STATIC			0x01
#define ZEND_ACC_ABSTRACT		0x02
#define ZEND_ACC_FINAL			0x04
#define ZEND_ACC_IMPLEMENTED_ABSTRACT		0x08

/* method flags (visibility) */
/* The order of those must be kept - public < protected < private */
#define ZEND_ACC_PUBLIC		0x100
#define ZEND_ACC_PROTECTED	0x200
#define ZEND_ACC_PRIVATE	0x400
#define ZEND_ACC_PPP_MASK  (ZEND_ACC_PUBLIC | ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE)

#define ZEND_ACC_CHANGED	0x800
#define ZEND_ACC_IMPLICIT_PUBLIC	0x1000

/* method flags (special method detection) */
#define ZEND_ACC_CTOR		0x2000
#define ZEND_ACC_DTOR		0x4000

/* method flag used by Closure::__invoke() */
#define ZEND_ACC_USER_ARG_INFO 0x80

/* method flag (bc only), any method that has this flag can be used statically and non statically. */
#define ZEND_ACC_ALLOW_STATIC	0x10000

/* shadow of parent's private method/property */
#define ZEND_ACC_SHADOW 0x20000

/* deprecation flag */
#define ZEND_ACC_DEPRECATED 0x40000

#define ZEND_ACC_CLOSURE              0x100000
#define ZEND_ACC_FAKE_CLOSURE         0x40
#define ZEND_ACC_GENERATOR            0x800000

#define ZEND_ACC_NO_RT_ARENA          0x80000

/* call through user function trampoline. e.g. __call, __callstatic */
#define ZEND_ACC_CALL_VIA_TRAMPOLINE  0x200000

/* call through internal function handler. e.g. Closure::invoke() */
#define ZEND_ACC_CALL_VIA_HANDLER     ZEND_ACC_CALL_VIA_TRAMPOLINE

/* disable inline caching */
#define ZEND_ACC_NEVER_CACHE          0x400000

#define ZEND_ACC_VARIADIC				0x1000000

#define ZEND_ACC_RETURN_REFERENCE		0x4000000
#define ZEND_ACC_DONE_PASS_TWO			0x8000000

/* class has magic methods __get/__set/__unset/__isset that use guards */
#define ZEND_ACC_USE_GUARDS				0x1000000

/* function has typed arguments */
#define ZEND_ACC_HAS_TYPE_HINTS			0x10000000

/* op_array has finally blocks */
#define ZEND_ACC_HAS_FINALLY_BLOCK		0x20000000

/* internal function is allocated at arena */
#define ZEND_ACC_ARENA_ALLOCATED		0x20000000

/* Function has a return type (or class has such non-private function) */
#define ZEND_ACC_HAS_RETURN_TYPE		0x40000000

/* op_array uses strict mode types */
#define ZEND_ACC_STRICT_TYPES			0x80000000

#define ZEND_ACC_IMPLICIT_ABSTRACT_CLASS	0x10
#define ZEND_ACC_EXPLICIT_ABSTRACT_CLASS	0x20
#define ZEND_ACC_INTERFACE		            0x40
#define ZEND_ACC_TRAIT						0x80
#define ZEND_ACC_ANON_CLASS                 0x100
#define ZEND_ACC_ANON_BOUND                 0x200
#define ZEND_ACC_INHERITED                  0x400

/* class implement interface(s) flag */
#define ZEND_ACC_IMPLEMENT_INTERFACES 0x80000
#define ZEND_ACC_IMPLEMENT_TRAITS	  0x400000

/* class constants updated */
#define ZEND_ACC_CONSTANTS_UPDATED	  0x100000

/* user class has methods with static variables */
#define ZEND_HAS_STATIC_IN_METHODS    0x800000

int lex_scan(zval*);
int zendlex(zend_parser_stack_elem*);
void startup_scanner();

static inline uint32_t zend_add_class_modifier(uint32_t flags, uint32_t new_flag) /* {{{ */
{
	uint32_t new_flags = flags | new_flag;
	if ((flags & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS) && (new_flag & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Multiple abstract modifiers are not allowed");
	}
	if ((flags & ZEND_ACC_FINAL) && (new_flag & ZEND_ACC_FINAL)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Multiple final modifiers are not allowed");
	}
	if ((new_flags & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS) && (new_flags & ZEND_ACC_FINAL)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Cannot use the final modifier on an abstract class");
	}
	return new_flags;
}
/* }}} */

static inline uint32_t zend_add_member_modifier(uint32_t flags, uint32_t new_flag) /* {{{ */
{
	uint32_t new_flags = flags | new_flag;
	if ((flags & ZEND_ACC_PPP_MASK) && (new_flag & ZEND_ACC_PPP_MASK)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Multiple access type modifiers are not allowed");
	}
	if ((flags & ZEND_ACC_ABSTRACT) && (new_flag & ZEND_ACC_ABSTRACT)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Multiple abstract modifiers are not allowed");
	}
	if ((flags & ZEND_ACC_STATIC) && (new_flag & ZEND_ACC_STATIC)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Multiple static modifiers are not allowed");
	}
	if ((flags & ZEND_ACC_FINAL) && (new_flag & ZEND_ACC_FINAL)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Multiple final modifiers are not allowed");
	}
	if ((new_flags & ZEND_ACC_ABSTRACT) && (new_flags & ZEND_ACC_FINAL)) {
		zend_error_noreturn(E_COMPILE_ERROR, "Cannot use the final modifier on an abstract class member");
	}
	return new_flags;
}
/* }}} */

void zend_handle_encoding_declaration(zend_ast*);


/*
 * Class flags
 *
 * Classes also use the ZEND_ACC_FINAL (0x04) flag, otherwise there is no overlap.
 */

/* class flags (types) */
/* ZEND_ACC_IMPLICIT_ABSTRACT_CLASS is used for abstract classes (since it is set by any abstract method even interfaces MAY have it set, too). */
/* ZEND_ACC_EXPLICIT_ABSTRACT_CLASS denotes that a class was explicitly defined as abstract by using the keyword. */
#define ZEND_ACC_IMPLICIT_ABSTRACT_CLASS	0x10
#define ZEND_ACC_EXPLICIT_ABSTRACT_CLASS	0x20
#define ZEND_ACC_INTERFACE		            0x40
#define ZEND_ACC_TRAIT						0x80
#define ZEND_ACC_ANON_CLASS                 0x100
#define ZEND_ACC_ANON_BOUND                 0x200
#define ZEND_ACC_INHERITED                  0x400

/* class implement interface(s) flag */
#define ZEND_ACC_IMPLEMENT_INTERFACES 0x80000
#define ZEND_ACC_IMPLEMENT_TRAITS	  0x400000

/* class constants updated */
#define ZEND_ACC_CONSTANTS_UPDATED	  0x100000

/* user class has methods with static variables */
#define ZEND_HAS_STATIC_IN_METHODS    0x800000

#define ZEND_RETURN_VAL 0
#define ZEND_RETURN_REF 1


#define ZEND_RETURNS_FUNCTION 1<<0
#define ZEND_RETURNS_VALUE    1<<1

#define ZEND_ARRAY_ELEMENT_REF		(1<<0)
#define ZEND_ARRAY_NOT_PACKED		(1<<1)
#define ZEND_ARRAY_SIZE_SHIFT		2

/* For "use" AST nodes and the seen symbol table */
#define ZEND_SYMBOL_CLASS    (1<<0)
#define ZEND_SYMBOL_FUNCTION (1<<1)
#define ZEND_SYMBOL_CONST    (1<<2)

/* Pseudo-opcodes that are used only temporarily during compilation */
#define ZEND_GOTO  253
#define ZEND_BRK   254
#define ZEND_CONT  255


////////////////////////////////////////////////////////////////////////////////
// zend_variables.h
//

void zval_ptr_dtor_nogc(zval*);
#define zval_ptr_dtor zval_ptr_dtor_nogc


////////////////////////////////////////////////////////////////////////////////
// zend_stream.h

#define ZEND_MMAP_AHEAD 32

////////////////////////////////////////////////////////////////////////////////
// zend_multibyte.h

typedef size_t (*zend_encoding_filter)(unsigned char **str, size_t *str_length, const unsigned char *buf, size_t length);


////////////////////////////////////////////////////////////////////////////////
// Compatibility with zend_globals.h

#define CG(name) zend_compat_state()->name
#define LANG_SCNG(name) zend_compat_state()->name

typedef enum {
	ON_TOKEN,
	ON_FEEDBACK,
	ON_STOP
} zend_php_scanner_event;

typedef struct _parser_state {
  zend_ast *ast;
	zend_arena *ast_arena;

  uint64_t zend_lineno;
  uint32_t extra_fn_flags;
  zend_string* doc_comment;
  zend_bool parse_error;

	zend_bool increment_lineno;
	zend_bool short_tags;

	unsigned int yy_leng;
	unsigned char *yy_start;
	unsigned char *yy_text;
	unsigned char *yy_cursor;
	unsigned char *yy_marker;
	unsigned char *yy_limit;
	int yy_state;
	zend_stack state_stack;
	zend_ptr_stack heredoc_label_stack;

	/* original (unfiltered) script */
	unsigned char *script_org;
	size_t script_org_size;

	/* input/output filters */
  zend_encoding_filter input_filter;
  zend_encoding_filter output_filter;

	/* initial string length after scanning to first variable */
	int scanned_string_len;

	/* hooks */
	void (*on_event)(zend_php_scanner_event event, int token, int line, void *context);
	void *on_event_context;


} parser_state;

parser_state* zend_compat_state();

void init_parser_state();
void destroy_parser_state();

////////////////////////////////////////////////////////////////////////////////
// zend_exceptions.h

NORETURN void zenderror(const char*);
NORETURN void zend_throw_exception(zend_class_entry* cls, const char* msg, zend_long code);
extern zend_class_entry* zend_ce_parse_error;

////////////////////////////////////////////////////////////////////////////////
// zend_operators.h

#define zend_memrchr memrchr


END_EXTERN_C()

#endif // ZEND_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
