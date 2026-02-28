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

namespace HPHP {

enum IMAGICK_COLOR {
  IMAGICK_COLOR_BLACK = 0,
  IMAGICK_COLOR_BLUE,
  IMAGICK_COLOR_CYAN,
  IMAGICK_COLOR_GREEN,
  IMAGICK_COLOR_RED,
  IMAGICK_COLOR_YELLOW,
  IMAGICK_COLOR_MAGENTA,
  IMAGICK_COLOR_OPACITY,
  IMAGICK_COLOR_ALPHA,
  IMAGICK_COLOR_FUZZ
};

extern const StaticString
  // coord
  s_x,
  s_y,
  // size
  s_columns,
  s_rows,
  // geometry
  s_width,
  s_height,
  // affine
  s_sx,
  s_rx,
  s_ry,
  s_sy,
  s_tx,
  s_ty,
  // color
  s_r,
  s_g,
  s_b,
  s_a,
  s_hue,
  s_saturation,
  s_luminosity,
  // extrema
  s_min,
  s_max,
  // channel extrema/range
  s_minima,
  s_maxima,
  // channel kurtosis
  s_kurtosis,
  s_skewness,
  // channel mean
  s_mean,
  s_standardDeviation,
  // other channel statistics
  s_depth,
  // quantum depth
  s_quantumDepthLong,
  s_quantumDepthString,
  // quantum range
  s_quantumRangeLong,
  s_quantumRangeString,
  // version
  s_versionNumber,
  s_versionString,
  // identify
  s_imageName,
  s_mimetype,
  s_geometry,
  s_resolution,
  s_signature,
  s_rawOutput,
  s_format,
  s_units,
  s_type,
  s_colorSpace,
  s_fileSize,
  s_compression,
  // font metrics
  s_characterWidth,
  s_characterHeight,
  s_ascender,
  s_descender,
  s_textWidth,
  s_textHeight,
  s_maxHorizontalAdvance,
  s_x1,
  s_y1,
  s_x2,
  s_y2,
  s_originX,
  s_originY,
  s_boundingBox,
  // class name
  s_Imagick,
  s_ImagickDraw,
  s_ImagickPixel,
  s_ImagickPixelIterator;

void registerNativeImagickConstants();

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
