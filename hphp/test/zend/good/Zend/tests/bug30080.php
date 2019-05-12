<?php
class foo {
  function foo($arrayobj) {
    var_dump($arrayobj);
  }
}
<<__EntryPoint>> function main() {
new foo(array(new stdClass));
}
