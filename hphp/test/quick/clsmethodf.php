<?php

class aa {
  protected function blah() {
    echo "protected aa::blah\n";
    var_dump($this);
  }

  protected function func($o) {
    echo "protected aa::blah\n";
    var_dump($o === $this);
  }

  public static function callfunc() {
    self::func(null);
    self::func(null);
  }

  public function __call($name, $args) {
    $args = count($args);
    echo "magic call to aa->$name with $args arguments\n";
  }
}

class a extends aa {
  protected function blah() {
    echo "private a::blah\n";
  }

  public static function stat() {
    echo "public static a::stat\n";
  }

  public function nons() {
    $str = 'blah';
    self::$str();
    self::stat();
    parent::blah();

    parent::func(null);
    parent::func($this);
    parent::func(null);
    parent::func($this);

    parent::callfunc();

    self::fakemethod(1, 2, 3);
    self::fakemethod();
  }
}

function main() {
  $a = new a();
  $a->nons();
}
main();
