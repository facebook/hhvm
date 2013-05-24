<?php

trait A {
  function b() {
    $c = function() {
      return 'd';
    };
    var_dump($c);
    return $c();
  }
}

class E { use A; }
class F { use A; }

function main() {
  var_dump((new E)->b());
  var_dump((new F)->b());
}

main();
