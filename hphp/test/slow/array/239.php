<?php

function foo($x) {
 var_dump($x);
 }
function test() {
  $data = null;
  $data['bar']['baz'] = 1;
  foo($data);
}

<<__EntryPoint>>
function main_239() {
test();
}
