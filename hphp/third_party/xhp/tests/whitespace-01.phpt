--TEST--
Whitespace 01
--FILE--
<?php
require 'lib.php';
echo <x>
<x>
</x>.
</x>;
--EXPECT--
<x><x></x>. </x>
