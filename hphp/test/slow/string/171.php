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
foo();
