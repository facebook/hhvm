<?php
class X {
  function f($x){var_dump($$x);}
}
$x = new X;
$x->f("this");
?>