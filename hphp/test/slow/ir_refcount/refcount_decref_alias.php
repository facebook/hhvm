<?php

class X {
  private $rc_prop = array();
  function __construct(array $x) {
    $this->rc_prop = $x;
  }
  function __destruct() {
    $this->rc_prop = null;
  }
  function thing() {
    return $this->rc_prop;
  }
}

function go() {
  var_dump((new X([new stdclass]))->thing());
  var_dump((new X([new stdclass]))->thing());
  var_dump((new X([new stdclass]))->thing());
  var_dump((new X([new stdclass]))->thing());
  var_dump((new X([new stdclass]))->thing());
}

go();
