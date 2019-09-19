<?hh


class C {
  const x = "C::x constant";
}

<<__EntryPoint>> function main(): void {
  print "Test begin\n";

  var_dump(C::x);
  $c = "C";
  var_dump($c::x);

  print "Test end\n";
}
