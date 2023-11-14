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

#include "hphp/runtime/ext/imagick/ext_imagick.h"

#include <vector>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

using std::pair;
using std::string;
using std::vector;

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////
// IO
bool isMagickPseudoFormat(const String& path, char mode /* = '*' */) {
  static const vector<pair<string, string>> pseudoFormats = {
    // Pseudo-image Formats
    {"CANVAS:",          "R"},
    {"CAPTION:",         "R"},
    {"CLIP:",            "RW"},
    {"CLIPBOARD:",       "RW"},
    {"FRACTAL:",         "R"},
    {"GRADIENT:",        "R"},
    {"HALD:",            "R"},
    {"HISTOGRAM:",        "W"},
    {"LABEL:",           "R"},
    {"MAP:",             "RW"},
    {"MASK:",            "RW"},
    {"MATTE:",            "W"},
    {"NULL:",            "RW"},
    {"PANGO:",           "R"},
    {"PLASMA:",          "R"},
    {"PREVIEW:",          "W"},
    {"PRINT:",            "W"},
    {"SCAN:",            "R"},
    {"RADIAL_GRADIENT:", "R"},
    {"SCANX:",           "R"},
    {"STEGANO:",         "R"},
    {"TILE:",            "R"},
    {"UNIQUE:",           "W"},
    {"VID:",             "RW"},
    {"WIN:",             "RW"},
    {"X:",               "RW"},
    {"XC:",              "R"},
    // Built-in Images
    {"MAGICK:",          "R"},
    {"GRANITE:",         "R"},
    {"LOGO:",            "R"},
    {"NETSCAPE:",        "R"},
    {"ROSE:",            "R"},
    // Built-in Patterns
    {"PATTERN:",         "R"},
  };
  for (const auto& i: pseudoFormats) {
    if (strncasecmp(path.c_str(), i.first.c_str(), i.first.length()) == 0) {
      return mode == '*' || i.second.find(mode) != string::npos;
    }
  }
  return false;
}

#define IMAGICK_THROW imagickThrow<ImagickException>

void imagickReadOp(MagickWand* wand,
                   const String& path,
                   const ImagickFileOp& op) {
  String realpath;
  if (isMagickPseudoFormat(path, 'R')) {
    realpath = path;
  } else {
    auto var = HHVM_FN(realpath)(path);
    realpath = var.isString() ? var.toString() : String();
    if (realpath.empty() ||
        !HHVM_FN(is_file)(realpath) ||
        !HHVM_FN(is_readable)(realpath)) {
      realpath.reset();
    }
  }
  if (realpath.empty()) {
    IMAGICK_THROW("Invalid filename provided");
  }

  auto status = op(wand, path.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to read image file");
  }
  MagickSetImageFilename(wand, realpath.c_str());
  MagickSetLastIterator(wand);
}

void imagickWriteOp(MagickWand* wand,
                    const String& path_,
                    const ImagickFileOp& op) {
  String path = path_;
  if (path.empty()) {
    path = convertMagickString(MagickGetImageFilename(wand));
  }
  if (path.empty()) {
    IMAGICK_THROW("No image filename specified");
  }

  String realpath;
  if (isMagickPseudoFormat(path, 'W')) {
    realpath = path;
  } else {
    static const int PHP_PATHINFO_DIRNAME = 1;
    String dirname = HHVM_FN(pathinfo)(path, PHP_PATHINFO_DIRNAME).toString();
    if (!HHVM_FN(is_dir)(dirname)) {
      // nothing to do here
    } else if (!HHVM_FN(is_file)(path)) {
      if (HHVM_FN(is_writable)(dirname)) {
        realpath = path;
      }
    } else {
      if (HHVM_FN(is_writable)(path)) {
        realpath = HHVM_FN(realpath)(path).toString();
      }
    }
  }
  if (realpath.empty()) {
    IMAGICK_THROW("Invalid filename provided");
  }

  auto status = op(wand, path.c_str());
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to write image file");
  }
}

ALWAYS_INLINE
FILE* getFILE(const OptResource& res, bool isWrite) {
  auto f = dyn_cast_or_null<File>(res);
  if (f == nullptr || f->isClosed()) {
    IMAGICK_THROW("Invalid stream resource");
  }

  FILE* fp;
  if (isWrite) {
    f->flush();
    fp = fdopen(dup(f->fd()), "ab");
  } else {
    fp = fdopen(dup(f->fd()), "rb");
    if (fp != nullptr && f->seekable()) {
      fseek(fp, f->tell(), SEEK_SET);
    }
  }
  if (fp == nullptr) {
    IMAGICK_THROW("Invalid stream resource");
  }

  return fp;
}

void imagickReadOp(MagickWand* wand,
                   const OptResource& res,
                   const ImagickHandleOp& op) {
  auto fp = getFILE(res, false);
  auto status = op(wand, fp);
  fclose(fp);
  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to read image filehandle");
  }
}

void imagickWriteOp(MagickWand* wand,
                    const OptResource& res,
                    const String& format,
                    const ImagickHandleOp& op) {
  auto fp = getFILE(res, true);

  // Get the current name
  String filename = convertMagickString(MagickGetImageFilename(wand));
  if (!format.empty()) {
    MagickSetImageFilename(wand, (format + ":").c_str());
  }

  auto status = op(wand, fp);
  fclose(fp);

  // Restore the original name after write
  if (!format.empty()) {
    MagickSetImageFilename(wand, filename.c_str());
  }

  if (status == MagickFalse) {
    IMAGICK_THROW("Unable to write image filehandle");
  }
}

#undef IMAGICK_THROW

//////////////////////////////////////////////////////////////////////////////
// Common Helper
void raiseDeprecated(const char* className, const char* methodName) {
  raise_deprecated("%s::%s method is deprecated and it's use should be avoided",
                   className, methodName);
}

void raiseDeprecated(const char* className,
                     const char* methodName,
                     const char* newClass,
                     const char* newMethod) {
  raise_deprecated("%s::%s is deprecated. %s::%s should be used instead",
                   className, methodName, newClass, newMethod);
}

String convertMagickString(char* &&str) {
  if (str == nullptr) {
    return String();
  } else {
    String ret(str);
    freeMagickMemory(str);
    return ret;
  }
}

String convertMagickData(size_t size, unsigned char* &data) {
  if (data == nullptr) {
    return String();
  } else {
    String ret((char*)data, size, CopyString);
    freeMagickMemory(data);
    return ret;
  }
}

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
  ret.reserve(array.size());
  IterateV(array.get(), [&](TypedValue v) {
    ret.push_back(tvCastToDouble(v));
  });
  return ret;
}

std::vector<PointInfo> toPointInfoArray(const Array& coordinates) {
  std::vector<PointInfo> ret(coordinates.size());
  int idx = 0;

  for (ArrayIter it(coordinates); it; ++it) {
    auto const element = it.secondValPlus();
    if (!isArrayLikeType(type(element))) {
      return {};
    }

    auto const coordinate = val(element).parr;
    if (coordinate->size() != 2) {
      return {};
    }

    for (ArrayIter jt(coordinate); jt; ++jt) {
      const String& key = jt.first().toString();
      double value = tvCastToDouble(jt.secondValPlus());
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
  Extension("imagick", "3.2.0b2", NO_ONCALL_YET) {
}

void ImagickExtension::moduleInit() {
  loadImagickConstants();
  loadImagickClass();
  loadImagickDrawClass();
  loadImagickPixelClass();
  loadImagickPixelIteratorClass();
  MagickWandGenesis();
}

void ImagickExtension::moduleShutdown() {
  MagickWandTerminus();
}

void ImagickExtension::threadInit() {
  IniSetting::Bind(this, IniSetting::Mode::Request,
                   "imagick.locale_fix", "0",
                   &s_ini_setting->m_locale_fix);
  IniSetting::Bind(this, IniSetting::Mode::Request,
                   "imagick.progress_monitor", "0",
                   &s_ini_setting->m_progress_monitor);
}

bool ImagickExtension::hasLocaleFix() {
  return s_ini_setting->m_locale_fix;
}

bool ImagickExtension::hasProgressMonitor() {
  return s_ini_setting->m_progress_monitor;
}

RDS_LOCAL(ImagickExtension::ImagickIniSetting,
                       ImagickExtension::s_ini_setting);

ImagickExtension s_imagick_extension;

HHVM_GET_MODULE(imagick)

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
