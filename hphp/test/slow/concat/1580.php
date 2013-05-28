<?php

function foo() {
 return "hello" . "\0" . "world
";
 }
function bar() {
  $s = foo();
  echo $s;
}
bar();
