<?php
// at least 64 patterns required to trigger the WuManberReplacement algorithm
$key = 'aaaa';
for ( $i = 0; $i < 66; ++$i ) {
  $map[$key++] = 'x';
}
// source string must have fewer bytes then the smallest pattern above
echo strtr( 'ab', $map ), "\n";
