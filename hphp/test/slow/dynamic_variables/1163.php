<?php

function test($a) {
  $b = 5;
  global $$a;
  var_dump($b);
}

<<__EntryPoint>>
function main_1163() {
test('b');
}
