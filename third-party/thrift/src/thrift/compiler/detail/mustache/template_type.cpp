/*

The source code contained in this file is based on the original code by
Daniel Sipka (https://github.com/no1msd/mstch). The original license by Daniel
Sipka can be read below:

The MIT License (MIT)

Copyright (c) 2015 Daniel Sipka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <thrift/compiler/detail/mustache/template_type.h>

namespace apache {
namespace thrift {
namespace mstch {

template_type::template_type(const std::string& str, const delim_type& delims)
    : m_open(delims.first), m_close(delims.second) {
  tokenize(str);
  strip_whitespace();
}

template_type::template_type(const std::string& str)
    : m_open("{{"), m_close("}}") {
  tokenize(str);
  strip_whitespace();
}

void template_type::process_text(citer begin, citer end) {
  if (begin == end) {
    return;
  }
  auto start = begin;
  for (auto it = begin; it != end; ++it) {
    if (*it == '\n' || it == end - 1) {
      m_tokens.push_back({{start, it + 1}});
      start = it + 1;
    }
  }
}

void template_type::tokenize(const std::string& tmp) {
  citer beg = tmp.begin();
  auto npos = std::string::npos;

  for (std::size_t cur_pos = 0; cur_pos < tmp.size();) {
    auto open_pos = tmp.find(m_open, cur_pos);
    auto close_pos =
        tmp.find(m_close, open_pos == npos ? open_pos : open_pos + 1);

    if (close_pos != npos && open_pos != npos) {
      process_text(beg + cur_pos, beg + open_pos);
      cur_pos = close_pos + m_close.size();
      m_tokens.push_back(
          {{beg + open_pos, beg + close_pos + m_close.size()},
           m_open.size(),
           m_close.size()});

      if (cur_pos == tmp.size()) {
        m_tokens.push_back({{""}});
        m_tokens.back().eol(true);
      }

      if (*(beg + open_pos + m_open.size()) == '=' &&
          *(beg + close_pos - 1) == '=') {
        auto tok_beg = beg + open_pos + m_open.size() + 1;
        auto tok_end = beg + close_pos - 1;
        auto front_skip = first_not_ws(tok_beg, tok_end);
        auto back_skip = first_not_ws(reverse(tok_end), reverse(tok_beg));
        m_open = {front_skip, beg + tmp.find(' ', front_skip - beg)};
        m_close = {beg + tmp.rfind(' ', back_skip - beg) + 1, back_skip + 1};
      }
    } else {
      process_text(beg + cur_pos, tmp.end());
      cur_pos = close_pos;
    }
  }
}

void template_type::strip_whitespace() {
  auto erases = std::vector<bool>();
  erases.resize(m_tokens.size());
  auto line_begin = m_tokens.begin();
  bool has_tag = false, non_space = false;

  for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it) {
    auto type = (*it).token_type();
    if (type != token::type::text && type != token::type::variable) {
      has_tag = true;
    } else if (!(*it).ws_only()) {
      non_space = true;
    }

    if ((*it).eol()) {
      if (has_tag && !non_space) {
        store_prefixes(line_begin);

        auto c = line_begin;
        for (bool end = false; !end; ++c) {
          if ((end = (*c).eol())) {
            it = c - 1;
          }
          if ((*c).ws_only()) {
            erases[c - m_tokens.begin()] = true;
          }
        }
      }

      non_space = has_tag = false;
      line_begin = it + 1;
    }
  }

  size_t compact = 0;
  for (size_t expanded = 0; expanded < m_tokens.size(); ++expanded) {
    if (!erases[expanded]) {
      std::swap(m_tokens[compact], m_tokens[expanded]);
      ++compact;
    }
  }
  m_tokens.erase(m_tokens.begin() + compact, m_tokens.end());
}

void template_type::store_prefixes(std::vector<token>::iterator beg) {
  for (auto cur = beg; !(*cur).eol(); ++cur) {
    if ((*cur).token_type() == token::type::partial && cur != beg &&
        (*(cur - 1)).ws_only()) {
      (*cur).partial_prefix((*(cur - 1)).raw());
    }
  }
}

} // namespace mstch
} // namespace thrift
} // namespace apache
