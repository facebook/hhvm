<?php
<<__Memoize>>
function test_top_level() { static $i = 100; return $i++; }

echo test_top_level().' ';
echo test_top_level();
