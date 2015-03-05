<?php


function main($num, $zero) {
  $z = $num / 0;
  $zz = $num / $zero;
  var_dump($z, $zz);
}
main(123, 0);
