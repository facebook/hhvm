<?php

class X {
}
function test($x) {
  switch (true) {
    case $x instanceof X: var_dump('X');
 break;
    default: var_dump('Other');
 break;
  }
}

<<__EntryPoint>>
function main_1760() {
test(new X);
}
