<?php
  echo "Begin\n";
  const THE_CONST = 123;
  function f($a=array(THE_CONST=>THE_CONST)) {
    print_r($a);
  }
  f();
  f();
  f();
  echo "Done";
