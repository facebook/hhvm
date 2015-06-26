<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class thing {
  public $a, $b = 'hi';
  public function __construct() {
    $this->a =& $this->b;
  }
}

function main() {
  $o = new thing;
  $local = $o?->a;
  $local = 5;
  var_dump($o->b);
}
main();
