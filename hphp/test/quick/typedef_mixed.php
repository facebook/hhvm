<?php

type Foo = mixed;

function foo(Foo $x) {
  echo $x;
  echo "\n";
}

foo(12);
