<?php

trait T {
  function foo() {
    var_dump(__METHOD__);
  }
}

function test($f) {
  call_user_func($f);
}


<<__EntryPoint>>
function main_classof() {
test(array('T', 'T::foo'));
}
