/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/shape.h"

namespace HPHP {

std::atomic<size_t> Shape::s_totalShapes;

Shape::Shape()
  : m_size(0)
  , m_capacity(0)
{
}

Shape::Shape(const Shape& from)
  : m_size(from.m_size)
  , m_capacity(from.m_capacity)
  , m_table(from.m_table)
{
}

Shape* Shape::emptyShape() {
  static Shape empty;
  return &empty;
}

Shape* Shape::create() {
  return new Shape();
}

Shape* Shape::clone(Shape* from) {
  return new Shape(*from);
}

std::string show(const Shape& shape) {
  auto ret = std::string{};
  ret += "{";
  assert(shape.size() >= 1);
  folly::format(&ret, "\"{}\"", shape.keyForOffset(0));
  for (uint32_t i = 1; i < shape.size(); ++i) {
    folly::format(&ret, ", \"{}\"", shape.keyForOffset(i));
  }
  ret += "}";
  return ret;
}

}
