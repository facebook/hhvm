--TEST--
Whitespace 04
--FILE--
<?php
require 'lib.php';
echo <x> <x /> {'a'} </x>;
--EXPECT--
<x><x></x>a</x>
