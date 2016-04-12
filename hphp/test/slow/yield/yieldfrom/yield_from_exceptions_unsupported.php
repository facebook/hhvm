<?php

function gen() {
  yield 1;
  yield 2;
  yield 3;
}

function yf() {
  yield from gen();
}

$g = yf();
// Prime the generator
$g->valid();
// THROW
$g->throw(new Exception("This should throw from gen!"));
