<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class wub {
  public static function wubwub($v) {
    return function() use ($v) { return $v; };
  }

  public static function wubwubwub() {
    return function() { return "I'm static"; };
  }
}

$fn = wub::wubwub('hello there');
var_dump($fn());
$fn = wub::wubwubwub();
var_dump($fn());
