/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/php7/ast_dump.h"
#include "hphp/php7/ast_info.h"

#include <folly/Format.h>

#include <ostream>

namespace HPHP { namespace php7 {

namespace {

const char* zend_ast_kind_name(zend_ast_kind kind) {
  switch (kind) {
#define BRANCH(name) case ZEND_AST_##name: return #name;
    AST_NODES(BRANCH)
#undef BRANCH
    default:
      return "?";
  }
}

void dump_ast_node_name(std::ostream& out, zend_ast* ast) {
  out << "(" << zend_ast_kind_name(ast->kind);
  if (ast->attr) {
    out << "[" << folly::format("{0:04x}", ast->attr) << "]";
  }
}

inline void do_indent(
    std::ostream& out,
    bool pretty,
    unsigned int indent) {
  if (pretty) {
    out << "\n";
    for (int i = 0; i <= indent; i++) {
      out << "  ";
    }
  } else {
    out << " ";
  }
}

void dump_zval(std::ostream& out, zval* zv) {
  switch (Z_TYPE_P(zv)) {
    case IS_LONG:
      out << Z_LVAL_P(zv);
      break;
    case IS_DOUBLE:
      out << Z_DVAL_P(zv);
      break;
    case IS_TRUE:
      out << "true";
      break;
    case IS_FALSE:
      out << "false ";
      break;
    case IS_NULL:
      out << "null ";
      break;
    case IS_STRING:
      out << "\"" << Z_STR_P(zv)->val << "\"";
      break;
    default:
      out << "<?>";
  }
}

} // namespace

void dump_ast(
    std::ostream& out,
    zend_ast* ast,
    bool pretty,
    unsigned int indent) {
  zend_ast_zval* zval_node;
  zend_ast_decl* decl;
  switch(ast->kind) {
    case ZEND_AST_ZVAL:
      zval_node = reinterpret_cast<zend_ast_zval*>(ast);
      dump_zval(out, &zval_node->val);
      break;
    case ZEND_AST_ZNODE:
      // still not sure what this one means
      out << "special";
      break;
    case ZEND_AST_FUNC_DECL:
    case ZEND_AST_CLOSURE:
    case ZEND_AST_METHOD:
    case ZEND_AST_CLASS:
      dump_ast_node_name(out, ast);
      decl = reinterpret_cast<zend_ast_decl*>(ast);
      do_indent(out, pretty, indent);
      if (decl->name) {
        out << decl->name->val;
      } else {
        out << "<anonymous>";
      }
      out << folly::format("[{:08x}]", decl->flags);
      for (int i = 0; i < 4; i++) {
        if (!decl->child[i]) {
          continue;
        }
        do_indent(out, pretty, indent);
        dump_ast(out, decl->child[i], pretty, indent + 1);
      }
      out << ")";
      break;
    default:
      if (ast->kind & (1 << ZEND_AST_IS_LIST_SHIFT)) {
        dump_ast_node_name(out, ast);
        zend_ast_list* list = reinterpret_cast<zend_ast_list*>(ast);
        for (int i = 0; i < list->children; i++) {
          do_indent(out, pretty, indent);
          if (!list->child[i])  {
            out << "<null>";
          } else {
            dump_ast(out, list->child[i], pretty, indent + 1);
          }
        }
        out << ")";
      } else {
        dump_ast_node_name(out, ast);
        size_t children = ast->kind >> ZEND_AST_NUM_CHILDREN_SHIFT;
        for (int i = 0; i < children; i++) {
          if (!ast->child[i]) {
            continue;
          }
          out << " ";
          dump_ast(out, ast->child[i], pretty, indent);
        }
        out << ")";
      }
  }
}

} } // HPHP::php7
