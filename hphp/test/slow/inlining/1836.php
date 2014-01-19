<?php

function id($a) {
 return $a;
 }
class X {
}
if (0) {
  class X {
}
}
class Y extends X {
 function t() {
}
 }
function test() {
  id(new Y)->t();
}
