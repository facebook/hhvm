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
foo();
foo();
