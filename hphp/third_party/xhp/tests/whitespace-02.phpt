--TEST--
Whitespace 02
--FILE--
<?php
require 'lib.php';
echo <x> {'a'}<x /></x>;
--EXPECT--
<x>a<x></x></x>
