<?php

function foo() {
  echo " FOO ";
  return " foo ";
}
function bar() {
  echo " hello " . foo() . "
";
  echo " hello " , foo() , "
";
}
bar();
