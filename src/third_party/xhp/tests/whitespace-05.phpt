--TEST--
Whitespace 05
--FILE--
<?php
require 'lib.php';
echo
  <x>
    foo
  </x>;
--EXPECT--
<x> foo </x>
