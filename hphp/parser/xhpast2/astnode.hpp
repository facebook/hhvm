/*
 * Copyright 2011 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>

#include "node_names.hpp"

#define NNEW(t) \
  (new xhpast::Node(t))

#define NTYPE(n, type) \
  ((n)->setType(type))

#define NMORE(n, end) \
  ((n)->setEnd(end))

#define NSPAN(n, type, end) \
  (NMORE(NTYPE((n), type), end))

#define NLMORE(n, begin) \
  ((n)->setBegin(begin))

#define NEXPAND(l, n, r) \
  ((n)->setBegin(l)->setEnd(r))


namespace xhpast {

  class Token;
  typedef std::list<Token *> token_list_t;

  class Token {

    public:
      unsigned int type;
      std::string value;
//      unsigned int lineno;
      unsigned int n;

      Token(unsigned int type, char *value, unsigned int n) :
        type(type),
        value(value),
        n(n) {
      }
  };

  class Node;
  typedef std::list<Node *> node_list_t;

  class Node {
    public:
      unsigned int type;

      int l_tok;
      int r_tok;

      node_list_t children;


      Node() : type(0), l_tok(-1), r_tok(-1) {};

      explicit Node(unsigned int type) : type(type), l_tok(-1), r_tok(-1) {};

      Node(unsigned int type, int end_tok) :
        type(type) {
          this->l_tok = end_tok;
          this->r_tok = end_tok;
      }

      Node(unsigned int type, int l_tok, int r_tok) :
        type(type),
        l_tok(l_tok),
        r_tok(r_tok) {

      }

      Node *appendChild(Node *node) {
        this->children.push_back(node);
        return this->setEnd(node);
      }

      Node *prependChild(Node *node) {
        this->children.push_front(node);
        return this->setBegin(node);
      }

      Node *appendChildren(Node *node) {
        for (node_list_t::iterator ii = node->children.begin();
             ii != node->children.end(); ++ii) {
          this->children.push_back(*ii);
          this->setEnd(*ii);
        }
        return this;
      }

      Node *firstChild() {
        if (this->children.empty()) {
          return nullptr;
        }
        return *(this->children.begin());
      }

      Node *setType(unsigned int t) {
        this->type = t;
        return this;
      }

      Node *setEnd(Node *n) {
        if (!n) {
          fprintf(stderr,
                  "Trying to setEnd() a null node to one of type %d\n",
                  this->type);
          exit(1);
        }

        if (n->r_tok != -1 && (n->r_tok > this->r_tok || (this->r_tok == -1))) {
          this->r_tok = n->r_tok;
        }
        if (this->l_tok == -1) {
          this->l_tok = n->l_tok;
        }
        return this;
      }

      Node *setBegin(Node *n) {
        if (!n) {
          fprintf(stderr,
                  "Trying to setBegin() a null node to one of type %d\n",
                  this->type);
          exit(1);
        }

        if (n->l_tok != -1 && (n->l_tok < this->l_tok || (this->l_tok == -1))) {
          this->l_tok = n->l_tok;
        }
        if (this->r_tok == -1) {
          this->r_tok = n->r_tok;
        }
        return this;
      }

  };
}
