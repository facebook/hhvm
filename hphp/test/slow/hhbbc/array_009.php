<?php

class Foo {
  private $thing;
  function go() {
    // This better not iterate the dataflow algorithm forever:
    $this->thing = array('thing' => $this->thing);
    return $this->thing;
  }
}

function main() {
  var_dump((new Foo)->go());
}

main();
