<?php


class C {
  const x = "C::x constant";
}

function main() {
  print "Test begin\n";

  var_dump(C::x);
  $c = "C";
  var_dump($c::x);

  print "Test end\n";
}
main();
