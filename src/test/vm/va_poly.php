<?

class C1 {
  function __call($a, $b) {
  }
}

class C2 {
  function __call($a, $b) {
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
    $o = new C2();
  }
}

main();
