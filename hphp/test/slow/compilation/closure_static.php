<?php
$foo = function () {
  static $a = Map {};
  if (!$a) $a['xxxxxxxxxxxxx'] = str_repeat('x', 2048);
  echo "hello\n";
};
$foo();
