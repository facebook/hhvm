<?php

class b {
  function z() {
    $this->x();
  }
  function y() {
    echo 'y';
  }
}
class c extends b {
  function x() {
    $this->y();
  }
}

<<__EntryPoint>>
function main_1481() {
if (false) {
  include '1481.inc';
}
$x = new c();
$x->z();
}
