<?hh

enum class E : mixed {
  int A = 42;
  string B = 'zuck';
}

<<__EntryPoint>>
function main(): void {
  echo "WRAPPED\n";
  var_dump(E#A);
  echo E#A; echo "\n";

  echo "\nUNWRAPPED\n";
  $unwrapped = __SystemLib\unwrap_opaque_value(0, E#A);
  var_dump($unwrapped);
  echo "$unwrapped\n";

  echo "\nVALUEOF\n";
  $valueof = E::valueOf(E#A);
  var_dump($valueof);

  echo "\nNAMEOF\n";
  $nameof = E::nameOf(E#A);
  var_dump($nameof);
  echo "$nameof\n";
}
