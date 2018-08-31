<?php

function foo() {
  if (!interface_exists('MyInterface')) {
    interface MyInterface{
}
;
    echo 'no';
  }
 else {
    echo 'yes';
  }
}

<<__EntryPoint>>
function main_1218() {
foo();
foo();
}
