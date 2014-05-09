<?php

function f(...$args) {}
function g($required, ...$args) {}
function h($optional = null, ...$args) {}

function test() {
  f();
  g(1);
  h();

  f(1,2,3,4);
  g(1,2,3,4);
  h(1,2,3,4);
}
test();
