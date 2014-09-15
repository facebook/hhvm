<?php

set_error_handler(
  function($errno, $str) {
    var_dump($str);
  }
);

class ReturnsString {
  public function __toString() {
    return self::class;
  }
}

var_dump((string) (new ReturnsString()));

class ReturnsThis {
  public function __toString() {
    return $this;
  }
}

var_dump((string) (new ReturnsThis()));

class ReturnsInt {
  public function __toString() {
    return 42;
  }
}

var_dump((string) (new ReturnsInt()));

class ReturnsNull {
  public function __toString() {
    return null;
  }
}

var_dump((string) (new ReturnsInt()));

class ReturnsStringable {
  public function __toString() {
    return new ReturnsString();
  }
}

var_dump((string) (new ReturnsStringable()));
