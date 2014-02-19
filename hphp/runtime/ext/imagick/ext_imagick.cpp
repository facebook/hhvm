/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/imagick/ext_imagick.h"

namespace HPHP {

HPHP::Class* ImagickException::cls = nullptr;
HPHP::Class* ImagickDrawException::cls = nullptr;
HPHP::Class* ImagickPixelException::cls = nullptr;
HPHP::Class* ImagickPixelIteratorException::cls = nullptr;

HPHP::Class* Imagick::cls = nullptr;
HPHP::Class* ImagickDraw::cls = nullptr;
HPHP::Class* ImagickPixel::cls = nullptr;
HPHP::Class* ImagickPixelIterator::cls = nullptr;

class ImagickExtension : public Extension {
 public:
  ImagickExtension() : Extension("imagick") {}
  virtual void moduleInit() {
    loadImagickConstants();
    loadImagickClass();
    loadImagickDrawClass();
    loadImagickPixelClass();
    loadImagickPixelIteratorClass();
    loadSystemlib();
  }
} s_imagick_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(imagick);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
