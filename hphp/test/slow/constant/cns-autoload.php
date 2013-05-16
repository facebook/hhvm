<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function def() { define('FOO', 1); }
function show($a,$b) { var_dump($a,$b); }
function test() {
  show(FOO, FOO);
}

fb_autoload_map(array('constant' => array(),
                      'failure' => 'def'), "");

test();
