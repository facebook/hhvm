/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <iostream>
#include <cstdlib>
#include <string.h>

#include "folly/String.h"

#include "hphp/parser/xhpast2/parser.h"

namespace HPHP { namespace HPHP_PARSER_NS {

  bool g_verifyMode = false;

}}

void print_node(xhpast::Node *node) {
  int l = -1;
  int r = -1;
  if (node->l_tok != -1) {
    l = node->l_tok;
  }

  if (l == -1) {
    printf("[%d]", node->type);
  } else {
    if (node->r_tok != -1) {
      r = node->r_tok;
    }

    printf("[%d, %d, %d", node->type, l, r);
    if (!node->children.empty()) {
      printf(", [");
      for (xhpast::node_list_t::iterator ii = node->children.begin();;) {
        print_node(*ii);
        if (++ii != node->children.end()) {
          printf(",");
        } else {
          break;
        }
      }
      printf("]");
    }
    printf("]");
  }
}

/*
 * This program parses a file with the hphp php parser, and dumps
 * every callback the parser makes to stdout.
 *
 * If a parse error occurs, it says why.
 */
int main(int argc, char** argv) try {
  if (argc >= 2 && !strcmp(argv[1], "--verify")) {
    HPHP::XHPAST2::g_verifyMode = true;
    --argc, ++argv;
  }

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " [--verify] filename\n";
    std::exit(1);
  }

  std::ifstream in(argv[1]);
  if (!in.is_open()) {
    std::cerr << argv[0] << ": couldn't open file: "
              << folly::errnoStr(errno).toStdString() << '\n';
  }

  std::cout << "1..1\n";

  try {
    using HPHP::Scanner;
    using HPHP::XHPAST2::Parser;
    Scanner scan(in, Scanner::AllowShortTags);
    Parser parser(scan, argv[1]);
    parser.parse();
    parser.coalesceTree();
    std::cout << parser.tree << std::endl;
    xhpast::Node* root = parser.outputXHPAST();
    std::vector<xhpast::Token *>* tokens = &(parser.m_listener.tokens);
    printf("{");
    printf("\"tree\":");
    if (root) {
      // Extend the right token for the root node to the end of the concrete
      // token stream. This ensure all tokens appear in the tree. If we don't
      // do this and the file ends in tokens which don't go to the parser (like
      // comments and whitespace) they won't be represented in the tree.
      root->r_tok = (tokens->size() - 1);
      print_node(root);
    } else {
      printf("null");
    }
    printf(",");
    printf("\"stream\":");
    printf("[");

    for (std::vector<xhpast::Token *>::iterator ii = tokens->begin();;) {
      printf("[%d, %d]", (*ii)->type, (int)(*ii)->value.length());
      if (++ii != tokens->end()) {
        printf(",");
        } else {
        break;
      }
    }

    printf("]");
    printf("}\n");
  } catch (const std::exception& e) {
    if (HPHP::XHPAST2::g_verifyMode) {
      std::cout << "not ";
    } else {
      throw;
    }
  }
  std::cout << "ok 1\n";
}

catch (const std::runtime_error& e) {
  std::cerr << argv[0] << ": " << e.what() << '\n';
  return 1;
}
