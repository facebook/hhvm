<?php

// With autoload, no warning.
$i = 0;
function __autoload($k) {
  if ($GLOBALS['i'] == 0) {
    class a {}
  }
  ++$GLOBALS['i'];
}
class_alias('a', 'b');
new b();

// With autoload failing to add it.
class_alias('c', 'd');

// With autoload disabled.
class_alias('e', 'f', false);
