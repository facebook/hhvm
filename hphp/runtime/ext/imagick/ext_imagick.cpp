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

#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////
// PHP Exceptions and Classes
HPHP::Class* ImagickException::cls = nullptr;
HPHP::Class* ImagickDrawException::cls = nullptr;
HPHP::Class* ImagickPixelException::cls = nullptr;
HPHP::Class* ImagickPixelIteratorException::cls = nullptr;

HPHP::Class* Imagick::cls = nullptr;
HPHP::Class* ImagickDraw::cls = nullptr;
HPHP::Class* ImagickPixel::cls = nullptr;
HPHP::Class* ImagickPixelIterator::cls = nullptr;

//////////////////////////////////////////////////////////////////////////////
// Common Helper
MagickBooleanType withMagickLocaleFix(
    const std::function<MagickBooleanType()>& lambda) {
  static const char* const IMAGICK_LC_NUMERIC_LOCALE = "C";

  if (!ImagickExtension::hasLocaleFix()) {
    return lambda();
  }

  const char* plocale = setlocale(LC_NUMERIC, nullptr);
  if (plocale == nullptr) {
    return lambda();
  }

  // Switch the locale to IMAGICK_LC_NUMERIC_LOCALE if imagick.locale_fix is on
  const std::string locale = plocale;
  setlocale(LC_NUMERIC, IMAGICK_LC_NUMERIC_LOCALE);
  auto ret = lambda();
  setlocale(LC_NUMERIC, locale.c_str());
  return ret;
}

std::vector<double> toDoubleArray(const Array& array) {
  std::vector<double> ret;
  for (ArrayIter it(array); it; ++it) {
    ret.push_back(it.secondRefPlus().toDouble());
  }
  return ret;
}

std::vector<PointInfo> toPointInfoArray(const Array& coordinates) {
  std::vector<PointInfo> ret(coordinates.size());
  int idx = 0;

  for (ArrayIter it(coordinates); it; ++it) {
    const Variant& element = it.secondRefPlus();
    if (!element.isArray()) {
      return {};
    }

    const Array& coordinate = element.toCArrRef();
    if (coordinate.size() != 2) {
      return {};
    }

    for (ArrayIter jt(coordinate); jt; ++jt) {
      const String& key = jt.first().toString();
      double value = jt.secondRefPlus().toDouble();
      if (key == s_x) {
        ret[idx].x = value;
      } else if (key == s_y) {
        ret[idx].y = value;
      } else {
        return {};
      }
    }
    ++idx;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// ImagickExtension
ImagickExtension::ImagickExtension() :
  Extension("imagick") {
}

void ImagickExtension::moduleInit() {
  loadImagickConstants();
  loadImagickClass();
  loadImagickDrawClass();
  loadImagickPixelClass();
  loadImagickPixelIteratorClass();
  loadSystemlib();
}

void ImagickExtension::threadInit() {
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   "imagick.locale_fix", "0",
                   &s_ini_setting->m_locale_fix);
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   "imagick.progress_monitor", "0",
                   &s_ini_setting->m_progress_monitor);
}

bool ImagickExtension::hasLocaleFix() {
  return s_ini_setting->m_locale_fix;
}

bool ImagickExtension::hasProgressMonitor() {
  return s_ini_setting->m_progress_monitor;
}

IMPLEMENT_THREAD_LOCAL(ImagickExtension::ImagickIniSetting,
                       ImagickExtension::s_ini_setting);

ImagickExtension s_imagick_extension;

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
