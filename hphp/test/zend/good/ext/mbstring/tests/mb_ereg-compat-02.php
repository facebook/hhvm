<?hh
<<__EntryPoint>> function main(): void {
  /* (counterpart: ext/standard/tests/reg/005.phpt) */
  $a="This is a nice and simple string";
  $registers = null;
  echo mb_ereg(".*(is).*(is).*",$a,inout $registers);
  echo "\n";
  echo $registers[0];
  echo "\n";
  echo $registers[1];
  echo "\n";
  echo $registers[2];
  echo "\n";
}
