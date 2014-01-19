<?php

error_reporting(E_ALL & ~E_NOTICE);
function foo($x) {
}
function z() {
  $yay = 1;
  $snarf = 2;
  foo(1,foo(1), $yay,$snarf);
}
z();
