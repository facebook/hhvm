<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function __main__($filename) {
  if (!is_file($filename)) {
    throw new Exception($filename.' isn\'t a regular file');
  }
  require_once $filename;
}
__main__($argv[1]);
