--TEST--
Whitespace 07
--FILE--
<?php
require 'lib.php';
echo <x> a{ 'b' }c </x>;
--EXPECT--
<x> abc </x>
