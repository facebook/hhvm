<?hh // strict

class X {
  function __construct() {
    if (false) {
      var_dump($this?->x); // parse error
    }
  }
}

echo "not reached";

new X;
