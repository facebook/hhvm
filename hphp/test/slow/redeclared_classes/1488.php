<?php

if (isset($g)) {
  class X {
 function foo() {
 var_dump(__METHOD__);
 }
 }
  class Y extends X {
}
}
 else {
  class X {
 function foo() {
 var_dump(__METHOD__);
 }
 }
  class Y {
}
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
