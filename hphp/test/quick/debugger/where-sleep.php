<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Sleeper {
  public $foo;

  public function __construct($x) {
    $this->foo = $x;
  }

  public function __sleep() {
    echo "sleep\n";
    bar($this);
    return array('foo');
  }

  public function __wakeup() {
    echo "wakeup\n";
  }
}

function bar($b) {
  var_dump($b);
}

bar(new Sleeper(1));
