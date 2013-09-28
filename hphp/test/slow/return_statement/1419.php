<?php

class q {
}
function g() {
  return null;
  return new q;
}
function f() {
  return;
  return new q;
}
var_dump(g());
var_dump(f());
