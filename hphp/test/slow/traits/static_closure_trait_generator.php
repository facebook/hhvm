<?php
class Foo { const Bar = 12; };

trait Yoyo {
  function cl($k) {
    return function() use ($k) {
      static $x = "asd";
      yield $x++ . "2\n";
      yield $x++ . "2\n";
      yield $x++ . "2\n";
    };
  }
}

class Blah {
  use Yoyo;
}

function main() {
  $k = (new Blah)->cl(1);
  foreach ($k() as $x) {
    echo $x . "\n";
  }
  foreach ($k() as $x) {
    echo $x . "\n";
  }
  $k = (new Blah)->cl(1);
  foreach ($k() as $x) {
    echo $x . "\n";
  }
}

main();
