<?php

function h8() {
  $arr = array(0,1,2,3,4);
  end(&$arr);
  next(&$arr);
  $arr2 = $arr;
  var_dump(current(&$arr2));
  var_dump(current(&$arr));
}

<<__EntryPoint>>
function main_257() {
h8();
}
