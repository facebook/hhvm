<?php

if (isset($g)) {
  include '1488-1.inc';
}
 else {
  include '1488-2.inc';
}
class Z extends Y {
  function foo() {
 var_dump(__METHOD__);
 }
  function bar() {
 X::foo();
 }
}
Z::bar();
