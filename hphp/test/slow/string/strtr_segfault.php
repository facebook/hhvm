<?php

// at least 64 patterns required to trigger the WuManberReplacement algorithm
<<__EntryPoint>>
function main_strtr_segfault() {
$key = 'aaaa';
$map = array();
for ( $i = 0; $i < 66; ++$i ) {
  $map[$key++] = 'x';
}
// source string must have fewer bytes then the smallest pattern above
echo strtr( 'ab', $map ), "\n";
}
