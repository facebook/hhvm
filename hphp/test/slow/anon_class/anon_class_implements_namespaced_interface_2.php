<?php

namespace NS2 {

  interface ArgInterface {}

  interface AnonInterface {
    public function fun(ArgInterface $arg);
  }
}

namespace NS1 {

  use NS2\AnonInterface;
  use NS2\ArgInterface;

  $anon = new class implements AnonInterface {
    public function fun(ArgInterface $arg) {
    }
  };

  var_dump($anon);
}
