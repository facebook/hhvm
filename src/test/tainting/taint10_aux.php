<?php
  function foo(){
    $a = "a weird string";
    fb_set_taint($a, 0x1);
    echo($a);
    return $a;
  }
