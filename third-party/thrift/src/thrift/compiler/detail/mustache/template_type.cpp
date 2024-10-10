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

namespace apache::thrift::mstch {

template_type::template_type(const std::string& str) {
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
    auto open_pos = tmp.find(kOpen, cur_pos);
    auto close_pos =
        tmp.find(kClose, open_pos == npos ? open_pos : open_pos + 1);

    if (close_pos != npos && open_pos != npos) {
      process_text(beg + cur_pos, beg + open_pos);
      cur_pos = close_pos + kClose.size();
      m_tokens.push_back(
          {{beg + open_pos, beg + close_pos + kClose.size()},
           kOpen.size(),
           kClose.size()});

      if (cur_pos == tmp.size()) {
        m_tokens.push_back({{""}});
        m_tokens.back().eol(true);
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

} // namespace apache::thrift::mstch
