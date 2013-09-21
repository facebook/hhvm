<?php
echo hash('tiger192,3', ''),"\n";
echo hash('tiger192,3', 'abc'),"\n";
echo hash('tiger192,3', str_repeat('a', 63)),"\n";
echo hash('tiger192,3', str_repeat('abc', 61)),"\n";
echo hash('tiger192,3', str_repeat('abc', 64)),"\n";
?>