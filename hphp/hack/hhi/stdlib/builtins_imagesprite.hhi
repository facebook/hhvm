<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class ImageSprite {
  public function __construct() {}
  public function addFile(
    $file,
    $options = null,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function addString(
    $id,
    $data,
    $options = null,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function addUrl(
    $url,
    $timeout_ms = 0,
    $Options = null,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function clear($paths = null): HH\FIXME\MISSING_RETURN_TYPE {}
  public function loadDims($block = false): HH\FIXME\MISSING_RETURN_TYPE {}
  public function loadImages($block = false): HH\FIXME\MISSING_RETURN_TYPE {}
  public function output(
    $output_file = null,
    $format = "png",
    $quality = 75,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function css(
    $css_namespace,
    $sprite_file = null,
    $output_file = null,
    $verbose = false,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  public function getErrors(): HH\FIXME\MISSING_RETURN_TYPE {}
  public function mapping(): HH\FIXME\MISSING_RETURN_TYPE {}
}
