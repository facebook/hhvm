#include "zend.h"
#include "zend_language_parser.h"

#include <iostream>
#include <cstdlib>

// handles parse errors
void zenderror(const char* msg) {
  std::cerr << "Parse error: " << msg << std::endl;
  std::abort();
}

void zend_handle_encoding_declaration(zend_ast* /*declare_list*/) {
  /* nothing :) */
}

zend_string* zend_string_init(const char* str, size_t len, int /*persistent*/) {
  zend_string* ret = (zend_string*)emalloc(sizeof(zend_string) + len);
  ret->len = len;
  memcpy(&ret->val, str, len);
  ret->val[len] = 0;
  return ret;
}

zend_string*
zend_string_extend(zend_string* str, size_t new_len, int /*persistent*/) {

  zend_string* ret = (zend_string*)erealloc(str, sizeof(zend_string) + new_len);
  ret->len = new_len;
  return ret;
}

void zend_string_release(zend_string* str) {
  efree(str);
}

void* emalloc(size_t n) {
  void* ret;
  if (!(ret = malloc(n))) {
    std::abort();
  }
  return ret;
}

void* ecalloc(size_t count, size_t base) {
  void* ret;
  if (!(ret = calloc(count, base))) {
    std::abort();
  }
  return ret;
}


void efree(void* ptr) { free(ptr); }

void* erealloc(void* ptr, size_t size) {
  void* ret;
  if (!(ret = realloc(ptr, size))) {
    std::abort();
  }
  return ret;
}

void* safe_erealloc(void* ptr, size_t count, size_t size, size_t offset) {
  int overflowed;
  size_t safe_size = zend_safe_address(count, size, offset, &overflowed);
  if (overflowed) {
    return nullptr;
  }
  return erealloc(ptr, safe_size);
}

char* estrndup(const char* str, size_t n) {
  size_t len = strnlen(str, n);
  char* new_str = (char*)ecalloc(sizeof(char), len + 1);
  memcpy(new_str, str, len);
  new_str[len] = 0;
  return new_str;
}

zend_class_entry* zend_ce_parse_error = (zend_class_entry*)42;
void zend_throw_exception(zend_class_entry* /*cls*/, const char* msg,
                          zend_long /*code*/) {
  std::cerr << msg << std::endl;
  std::abort();
}

void zend_error(int /*type*/, const char* fmt, ...) {
  va_list argp;
  va_start(argp, fmt);

  vfprintf(stderr, fmt, argp);
}

void zend_error_noreturn(int /*type*/, const char* fmt, ...) {
  va_list argp;
  va_start(argp, fmt);

  vfprintf(stderr, fmt, argp);
  std::abort();
}

static thread_local parser_state* state_ = nullptr;
parser_state* zend_compat_state(){
  return state_;
}

void init_parser_state() {
  if (state_) { free(state_); }
  state_ = (parser_state*)emalloc(sizeof(parser_state));
  memset(state_, 0, sizeof(parser_state));
}

void destroy_parser_state() {
  if (state_) { efree(state_); }
  state_ = nullptr;
}

void zval_ptr_dtor_nogc(zval* zv) {
  switch (Z_TYPE_P(zv)) {
    case IS_STRING:
      zend_string_release(Z_STR_P(zv));
      break;
    case IS_LONG:
    case IS_DOUBLE:
    case IS_FALSE:
    case IS_TRUE:
    case IS_NULL:
      break; /* nothing */
    default:
      std::cerr << "weird zval?" << std::endl;
      std::abort();
  }

}


////////////////////////////////////////////////////////////////////////////////
// lifted from zend_compile.c

int lex_scan(zval*);
int zendlex(zend_parser_stack_elem *elem) /* {{{ */
{
	zval zv;
	int retval;
	uint32_t start_lineno;

	if (CG(increment_lineno)) {
		CG(zend_lineno)++;
		CG(increment_lineno) = 0;
	}

again:
	ZVAL_UNDEF(&zv);
	start_lineno = CG(zend_lineno);
	retval = lex_scan(&zv);

	switch (retval) {
		case T_COMMENT:
		case T_DOC_COMMENT:
		case T_OPEN_TAG:
		case T_WHITESPACE:
			goto again;

		case T_CLOSE_TAG:
			retval = ';'; /* implicit ; */
			break;
		case T_OPEN_TAG_WITH_ECHO:
			retval = T_ECHO;
			break;
	}
	if (Z_TYPE(zv) != IS_UNDEF) {
		elem->ast = zend_ast_create_zval_with_lineno(&zv, 0, start_lineno);
	}

	return retval;
}
/* }}} */

zend_ast *zend_negate_num_string(zend_ast *ast) /* {{{ */
{
	zval *zv = zend_ast_get_zval(ast);
	if (Z_TYPE_P(zv) == IS_LONG) {
		if (Z_LVAL_P(zv) == 0) {
			ZVAL_NEW_STR(zv, zend_string_init("-0", sizeof("-0")-1, 0));
		} else {
			ZEND_ASSERT(Z_LVAL_P(zv) > 0);
			Z_LVAL_P(zv) *= -1;
		}
	} else if (Z_TYPE_P(zv) == IS_STRING) {
		size_t orig_len = Z_STRLEN_P(zv);
		Z_STR_P(zv) = zend_string_extend(Z_STR_P(zv), orig_len + 1, 0);
		memmove(Z_STRVAL_P(zv) + 1, Z_STRVAL_P(zv), orig_len + 1);
		Z_STRVAL_P(zv)[0] = '-';
	} else {
		ZEND_ASSERT(0);
	}
	return ast;
}
/* }}} */

zend_ast *zend_ast_append_str(zend_ast *left_ast, zend_ast *right_ast) /* {{{ */
{
	zval *left_zv = zend_ast_get_zval(left_ast);
	zend_string *left = Z_STR_P(left_zv);
	zend_string *right = zend_ast_get_str(right_ast);

	zend_string *result;
	size_t left_len = ZSTR_LEN(left);
	size_t len = left_len + ZSTR_LEN(right) + 1; /* left\right */

	result = zend_string_extend(left, len, 0);
	ZSTR_VAL(result)[left_len] = '\\';
	memcpy(&ZSTR_VAL(result)[left_len + 1], ZSTR_VAL(right), ZSTR_LEN(right));
	ZSTR_VAL(result)[len] = '\0';
	zend_string_release(right);

	ZVAL_STR(left_zv, result);
	return left_ast;
}
/* }}} */
