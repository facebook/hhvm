<?php
echo hash('gost', ''), "\n";
echo hash('gost', 'The quick brown fox jumps over the lazy dog'), "\n";
echo hash('gost', 'The quick brown fox jumps over the lazy cog'), "\n";
echo hash('gost', str_repeat('a', 31)), "\n";
echo hash('gost', str_repeat('a', 32)), "\n";
echo hash('gost', str_repeat('a', 33)), "\n";
?>