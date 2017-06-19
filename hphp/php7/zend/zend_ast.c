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
   | Authors: Bob Weinand <bwoebi@php.net>                                |
   |          Dmitry Stogov <dmitry@zend.com>                             |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend_ast.h"
#include "zend_language_parser.h"

ZEND_API zend_ast_process_t zend_ast_process = NULL;

static inline void *zend_ast_alloc(size_t size) {
	return zend_arena_alloc(&CG(ast_arena), size);
}

static inline void *zend_ast_realloc(void *old, size_t old_size, size_t new_size) {
	void *new = zend_ast_alloc(new_size);
	memcpy(new, old, old_size);
	return new;
}

static inline size_t zend_ast_size(uint32_t children) {
	return sizeof(zend_ast) - sizeof(zend_ast *) + sizeof(zend_ast *) * children;
}

static inline size_t zend_ast_list_size(uint32_t children) {
	return sizeof(zend_ast_list) - sizeof(zend_ast *) + sizeof(zend_ast *) * children;
}

ZEND_API zend_ast *zend_ast_create_zval_with_lineno(zval *zv, zend_ast_attr attr, uint32_t lineno) {
	zend_ast_zval *ast;

	ast = zend_ast_alloc(sizeof(zend_ast_zval));
	ast->kind = ZEND_AST_ZVAL;
	ast->attr = attr;
	ZVAL_COPY_VALUE(&ast->val, zv);
	ast->val.u2.lineno = lineno;
	return (zend_ast *) ast;
}

ZEND_API zend_ast *zend_ast_create_zval_ex(zval *zv, zend_ast_attr attr) {
	return zend_ast_create_zval_with_lineno(zv, attr, CG(zend_lineno));
}

ZEND_API zend_ast *zend_ast_create_decl(
	zend_ast_kind kind, uint32_t flags, uint32_t start_lineno, zend_string *doc_comment,
	zend_string *name, zend_ast *child0, zend_ast *child1, zend_ast *child2, zend_ast *child3
) {
	zend_ast_decl *ast;

	ast = zend_ast_alloc(sizeof(zend_ast_decl));
	ast->kind = kind;
	ast->attr = 0;
	ast->start_lineno = start_lineno;
	ast->end_lineno = CG(zend_lineno);
	ast->flags = flags;
	ast->lex_pos = LANG_SCNG(yy_text);
	ast->doc_comment = doc_comment;
	ast->name = name;
	ast->child[0] = child0;
	ast->child[1] = child1;
	ast->child[2] = child2;
	ast->child[3] = child3;

	return (zend_ast *) ast;
}

static zend_ast *zend_ast_create_from_va_list(zend_ast_kind kind, zend_ast_attr attr, va_list va) {
	uint32_t i, children = kind >> ZEND_AST_NUM_CHILDREN_SHIFT;
	zend_ast *ast;

	ast = zend_ast_alloc(zend_ast_size(children));
	ast->kind = kind;
	ast->attr = attr;
	ast->lineno = (uint32_t) -1;

	for (i = 0; i < children; ++i) {
		ast->child[i] = va_arg(va, zend_ast *);
		if (ast->child[i] != NULL) {
			uint32_t lineno = zend_ast_get_lineno(ast->child[i]);
			if (lineno < ast->lineno) {
				ast->lineno = lineno;
			}
		}
	}

	if (ast->lineno == UINT_MAX) {
		ast->lineno = CG(zend_lineno);
	}

	return ast;
}

ZEND_API zend_ast *zend_ast_create_ex(zend_ast_kind kind, zend_ast_attr attr, ...) {
	va_list va;
	zend_ast *ast;

	va_start(va, attr);
	ast = zend_ast_create_from_va_list(kind, attr, va);
	va_end(va);

	return ast;
}

ZEND_API zend_ast *zend_ast_create(zend_ast_kind kind, ...) {
	va_list va;
	zend_ast *ast;

	va_start(va, kind);
	ast = zend_ast_create_from_va_list(kind, 0, va);
	va_end(va);

	return ast;
}

ZEND_API zend_ast *zend_ast_create_list(uint32_t init_children, zend_ast_kind kind, ...) {
	zend_ast *ast;
	zend_ast_list *list;

	ast = zend_ast_alloc(zend_ast_list_size(4));
	list = (zend_ast_list *) ast;
	list->kind = kind;
	list->attr = 0;
	list->lineno = CG(zend_lineno);
	list->children = 0;

	{
		va_list va;
		uint32_t i;
		va_start(va, kind);
		for (i = 0; i < init_children; ++i) {
			zend_ast *child = va_arg(va, zend_ast *);
			ast = zend_ast_list_add(ast, child);
			if (child != NULL) {
				uint32_t lineno = zend_ast_get_lineno(child);
				if (lineno < ast->lineno) {
					ast->lineno = lineno;
				}
			}
		}
		va_end(va);
	}

	return ast;
}

static inline zend_bool is_power_of_two(uint32_t n) {
	return ((n != 0) && (n == (n & (~n + 1))));
}

ZEND_API zend_ast *zend_ast_list_add(zend_ast *ast, zend_ast *op) {
	zend_ast_list *list = zend_ast_get_list(ast);
	if (list->children >= 4 && is_power_of_two(list->children)) {
			list = zend_ast_realloc(list,
			zend_ast_list_size(list->children), zend_ast_list_size(list->children * 2));
	}
	list->child[list->children++] = op;
	return (zend_ast *) list;
}

ZEND_API zend_ast *zend_ast_copy(zend_ast *ast)
{
	if (ast == NULL) {
		return NULL;
	} else if (ast->kind == ZEND_AST_ZVAL) {
		zend_ast_zval *new = emalloc(sizeof(zend_ast_zval));
		new->kind = ZEND_AST_ZVAL;
		new->attr = ast->attr;
		ZVAL_COPY(&new->val, zend_ast_get_zval(ast));
		return (zend_ast *) new;
	} else if (zend_ast_is_list(ast)) {
		zend_ast_list *list = zend_ast_get_list(ast);
		zend_ast_list *new = emalloc(zend_ast_list_size(list->children));
		uint32_t i;
		new->kind = list->kind;
		new->attr = list->attr;
		new->children = list->children;
		for (i = 0; i < list->children; i++) {
			new->child[i] = zend_ast_copy(list->child[i]);
		}
		return (zend_ast *) new;
	} else {
		uint32_t i, children = zend_ast_get_num_children(ast);
		zend_ast *new = emalloc(zend_ast_size(children));
		new->kind = ast->kind;
		new->attr = ast->attr;
		for (i = 0; i < children; i++) {
			new->child[i] = zend_ast_copy(ast->child[i]);
		}
		return new;
	}
}

static void zend_ast_destroy_ex(zend_ast *ast, zend_bool free) {
	if (!ast) {
		return;
	}

	switch (ast->kind) {
		case ZEND_AST_ZVAL:
			/* Destroy value without using GC: When opcache moves arrays into SHM it will
			 * free the zend_array structure, so references to it from outside the op array
			 * become invalid. GC would cause such a reference in the root buffer. */
			zval_ptr_dtor_nogc(zend_ast_get_zval(ast));
			break;
		case ZEND_AST_FUNC_DECL:
		case ZEND_AST_CLOSURE:
		case ZEND_AST_METHOD:
		case ZEND_AST_CLASS:
		{
			zend_ast_decl *decl = (zend_ast_decl *) ast;
			if (decl->name) {
			    zend_string_release(decl->name);
			}
			if (decl->doc_comment) {
				zend_string_release(decl->doc_comment);
			}
			zend_ast_destroy_ex(decl->child[0], free);
			zend_ast_destroy_ex(decl->child[1], free);
			zend_ast_destroy_ex(decl->child[2], free);
			zend_ast_destroy_ex(decl->child[3], free);
			break;
		}
		default:
			if (zend_ast_is_list(ast)) {
				zend_ast_list *list = zend_ast_get_list(ast);
				uint32_t i;
				for (i = 0; i < list->children; i++) {
					zend_ast_destroy_ex(list->child[i], free);
				}
			} else {
				uint32_t i, children = zend_ast_get_num_children(ast);
				for (i = 0; i < children; i++) {
					zend_ast_destroy_ex(ast->child[i], free);
				}
			}
	}

	if (free) {
		efree(ast);
	}
}

ZEND_API void zend_ast_destroy(zend_ast *ast) {
	zend_ast_destroy_ex(ast, 0);
}
ZEND_API void zend_ast_destroy_and_free(zend_ast *ast) {
	zend_ast_destroy_ex(ast, 1);
}

ZEND_API void zend_ast_apply(zend_ast *ast, zend_ast_apply_func fn) {
	if (zend_ast_is_list(ast)) {
		zend_ast_list *list = zend_ast_get_list(ast);
		uint32_t i;
		for (i = 0; i < list->children; ++i) {
			fn(&list->child[i]);
		}
	} else {
		uint32_t i, children = zend_ast_get_num_children(ast);
		for (i = 0; i < children; ++i) {
			fn(&ast->child[i]);
		}
	}
}

/*
 * Operator Precendence
 * ====================
 * priority  associativity  operators
 * ----------------------------------
 *   10     left            include, include_once, eval, require, require_once
 *   20     left            ,
 *   30     left            or
 *   40     left            xor
 *   50     left            and
 *   60     right           print
 *   70     right           yield
 *   80     right           =>
 *   85     right           yield from
 *   90     right           = += -= *= /= .= %= &= |= ^= <<= >>= **=
 *  100     left            ? :
 *  110     right           ??
 *  120     left            ||
 *  130     left            &&
 *  140     left            |
 *  150     left            ^
 *  160     left            &
 *  170     non-associative == != === !==
 *  180     non-associative < <= > >= <=>
 *  190     left            << >>
 *  200     left            + - .
 *  210     left            * / %
 *  220     right           !
 *  230     non-associative instanceof
 *  240     right           + - ++ -- ~ (type) @
 *  250     right           **
 *  260     left            [
 *  270     non-associative clone new
 */
