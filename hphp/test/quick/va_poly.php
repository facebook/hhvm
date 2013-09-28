<?

class C1 {
  function __call($a, $b) {
    echo '.';
  }
}

class C2 {
  function __call($a, $b) {
    echo '+';
  }
}

class C3 {
  function maul($a,$b,&$c) {
    echo '*';
    $c++;
  }
}

function main() {
  $o = new C1();
  // First pass through, "learn" that these calls go to C1::__call.
  for ($i = 0; $i < 12; $i++) {
    $o->maul(1, 2, $i, 3);
    $o->maul(1, 2, $i, 3, 4);
    $o->maul(1, 2, $i, 3, 4, 5);
    $o->maul(1, 2, $i, 3, 4, 5, 6);
    $o->maul(1, 2, $i, 3, 4, 5, 6, 7);
    $o->maul(1, 2, $i, 3, 4, 5, 6, 7, 8);
    // Send subsequent passes to C2::__call
    $o = $i == 0 ? new C2() : new C3();
    echo "\n";
  }
}

main();
