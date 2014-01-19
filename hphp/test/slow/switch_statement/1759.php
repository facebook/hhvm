<?php

function id($x) {
 return $x;
 }
function ret_false($x) {
 return false;
 }
function f($x) {
  switch ($x) {
  case ret_false($x = 32);
 echo 'fail';
 break;
  case id($x = 5): echo 'here';
 break;
  default: echo 'default';
  }
}
f(32);
