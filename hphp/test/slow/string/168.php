<?php

$s = 'x';
var_dump(strrpos($s.'0', $s));
for ($i = -7;
 $i < 7;
 $i++) {
  echo $i,':';
var_dump(strrpos('xabcay', 'a',$i));
}
