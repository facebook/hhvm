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
if (false) {
  class b{
}
  class c{
}
}
$x = new c();
$x->z();
