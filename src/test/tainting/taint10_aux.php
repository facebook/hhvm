<?php
  function foo(){
    $a = "a weird string";
    fb_taint($a);
    echo($a);
    return $a;
  }
