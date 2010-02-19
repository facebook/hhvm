/*
  +----------------------------------------------------------------------+
  | XHP                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2009 - 2010 Facebook, Inc. (http://www.facebook.com)          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE.PHP, and is    |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

#include <ext/rope>
#include <ext/pool_allocator.h>

class code_rope {
  public:
    typedef __gnu_cxx::rope<char, __gnu_cxx::__pool_alloc<char> > _rope_t;
  protected:
    _rope_t str;
    size_t lf; /* how many line breaks this code contains */
    size_t no; /* line number this code starts on */
  
  public:
    code_rope(const _rope_t = "", const size_t = 0, const size_t = 0);
    code_rope(const code_rope&, const size_t = 0, const size_t = 0);
    const char* c_str() const;
    void prepend(const char* str);
    const char back() const;
    void pop_back();
    void strip_lines();
    size_t lineno() const;
    code_rope operator+(const code_rope& right) const;
    code_rope operator+(const char*) const;
    code_rope& operator=(const char*);

  friend code_rope operator+(const char*, const code_rope&);
};
code_rope operator+(const char*, const code_rope&);
