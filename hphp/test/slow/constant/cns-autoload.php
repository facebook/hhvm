<?php

// Copyright 2004-2013 Facebook. All Rights Reserved.

function def() {
 define('FOO', 1);
 }
function show($a,$b) {
 var_dump($a,$b);
 }
function test() {
  show(FOO, FOO);
}

HH\autoload_set_paths(array('constant' => array(),
                               'failure' => 'def'), "");

test();
