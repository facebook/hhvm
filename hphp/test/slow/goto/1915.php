<?php

function fcn() {
 return true;
 }
class X {
  function f($x) {
    goto over_switch;
    switch ($this) {
    case fcn(): echo 'fcn';
    default: echo 'fun';
    }
    over_switch: var_dump($x);
  }
}
$x = new X;
$x->f(42);
