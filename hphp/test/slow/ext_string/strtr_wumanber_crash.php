<?php
$replace = [];

// Make sure to have more than 64 elements in the array so the
// Wu-Manber algorithm is used
for ($i = 0; $i < 65; ++$i) {
  // have every element be larger than the input size
  $replace["aa" . $i] = "aa" . $i;
}

echo strtr("aa",$replace);

