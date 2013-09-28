<?php

function comp_func($n1,$n2) {
  $n1=(int)$n1; $n2=(int)$n2;
  return $n1 === $n2 ? 0 : ($n1 > $n2 ? 1 : -1);
}


function a() {
  $array1 = array(
    "a" => "green",
    "b" => "brown",
    "c" => "blue",
    "red"
  );
  $array2 = array(
    "a" => "green",
    "yellow",
    "red"
  );

  $result = array_diff_uassoc($array1, $array2, "comp_func");
  var_dump($result);
}

a();
