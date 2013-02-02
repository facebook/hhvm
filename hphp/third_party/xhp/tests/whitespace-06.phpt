--TEST--
Whitespace 06
--FILE--
<?php
require 'lib.php';
echo <x> a { 'b' } c </x>;
--EXPECT--
<x> a b c </x>
