<?php
class X { use IterableTrait; }
function test() {
  $x = new X;
  var_dump($x->view());
}
test();


