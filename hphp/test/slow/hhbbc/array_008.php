<?php

function get() {
  return array(
    'foo' => 'bar',
    'baz' => 24,
    13 => 42,
    'heh' => 1.0
  );
}

function a() { return get()['foo']; }
function b() { return get()['baz']; }
function c() { return get()[13]; }
function d() { return get()['13']; }
function e() { return get()['13foo']; }

function main() {
  var_dump(a() === 'bar');
  var_dump(b() === 24);
  var_dump(c() === 42);
  var_dump(d() === 42);
  var_dump(e() === null);
}
main();
