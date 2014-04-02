<?php
class base {
  function base() {
    echo __CLASS__."::".__FUNCTION__."\n";
  }
}

class derived extends base {
  function base() {
    echo __CLASS__."::".__FUNCTION__."\n";
  }
}

function main() {
  $obj = new derived();
}

main();
