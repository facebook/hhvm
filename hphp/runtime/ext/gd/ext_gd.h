/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#pragma once


#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/zend-php-config.h"

using gdImagePtr = struct gdImage *;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct gfxinfo {
  unsigned int width;
  unsigned int height;
  unsigned int bits;
  unsigned int channels;
};

struct Image : SweepableResourceData {
  Image() : m_gdImage(nullptr) {}
  explicit Image(gdImagePtr gdImage) : m_gdImage(gdImage) {}
  ~Image() { sweep(); }
  gdImagePtr get() { return m_gdImage;}
  void reset();
  void sweep() { reset(); }

  CLASSNAME_IS("gd")
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }
  bool isInvalid() const override { return m_gdImage == nullptr; }

  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(Image)

  req::ptr<Image> m_brush;
  req::ptr<Image> m_tile;
private:
  gdImagePtr m_gdImage;
};

///////////////////////////////////////////////////////////////////////////////
}
