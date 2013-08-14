<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function id($x) { return $x; }

class c {}

function main() {
  $name = 'varname';

  $x = $undef['foo'];

  $x = $GLOBALS[$name]['foo'];
  $x = $GLOBALS[id($name)]['foo'];

  $x = ($$name)['foo'];
  $x = (${id('varname')})['foo'];
}
main();
