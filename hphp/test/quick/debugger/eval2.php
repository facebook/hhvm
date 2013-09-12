<?php
  function foo($x) {
    return $x + 1;
  }

  function throwSomething() {
    global $_;
    throw new Exception($_);
  }

  function printSomething() {
    global $_;
    echo $_;
    return "also returned something";
  }

