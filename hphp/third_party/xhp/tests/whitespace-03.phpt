--TEST--
Whitespace 03
--FILE--
<?php
require 'lib.php';
echo <x> {'a'} </x>;
--EXPECT--
<x>a</x>
