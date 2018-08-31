<?php

function foo() {
  $a = '';
  $a++;
  var_dump($a);
  $a = '';
  ++$a;
  var_dump($a);
  $a = '';
  $a--;
  var_dump($a);
  $a = '';
  --$a;
  var_dump($a);
  $a = '@';
  $a++;
  var_dump($a);
  $a = '@';
  ++$a;
  var_dump($a);
  $a = '@';
  $a--;
  var_dump($a);
  $a = '@';
  --$a;
  var_dump($a);
}

<<__EntryPoint>>
function main_171() {
foo();
}
