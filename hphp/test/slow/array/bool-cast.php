<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function f($a) {
  var_dump((bool)$a);
}

f($GLOBALS);
f(array('a' => 'b'));
f(array('a'));
f(array());
