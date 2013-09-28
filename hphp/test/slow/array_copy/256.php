<?php

function h7() {
  $arr = array(0,1,2,3,4);
  end($arr);
  next($arr);
  $arr2 = $arr;
  var_dump(current($arr));
  var_dump(current($arr2));
}
h7();
