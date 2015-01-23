<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
class ImageSprite {
  public function __construct() { }
  public function addFile($file, $options = null) { }
  public function addString($id, $data, $options = null) { }
  public function addUrl($url, $timeout_ms = 0, $Options = null) { }
  public function clear($paths = null) { }
  public function loadDims($block = false) { }
  public function loadImages($block = false) { }
  public function output($output_file = null, $format = "png", $quality = 75) { }
  public function css($css_namespace, $sprite_file = null, $output_file = null, $verbose = false) { }
  public function getErrors() { }
  public function mapping() { }
  public function __destruct() { }
}
