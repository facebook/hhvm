<?php

class B {
  function __call($name, $arguments) {
    echo "Calling B object method '$name' " . implode(', ', $arguments). "
";
  }
}
class G extends B {
  function __call($name, $arguments) {
    echo "Calling G object method '$name' " . implode(', ', $arguments). "
";
  }
  function f4missing($a) {
    $b="B";
    echo "Calling G object method 'f4missing' 5 == ", $b::f4missing(5);
  }
}

$g = new G();
$g->f4missing(3);
echo "finish
";
