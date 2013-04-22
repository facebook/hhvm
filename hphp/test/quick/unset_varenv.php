<?php

function foo() {
  $x = 'c';
  $$x = "haha";
  echo "value: ", $$x . "\n";
  unset($$x);
  echo "value: ", $$x . "\n";
}

foo();
