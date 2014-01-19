<?php

const FOO = 25;
const BAR = 42;

function foo() {
  return BAR % FOO;
}

var_dump(foo());
