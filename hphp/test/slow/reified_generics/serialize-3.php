<?hh

class D<reify T> {
  private int $z = 1;
}

class C extends D<(int, int)> {
  private int $x = 1;
  private int $y = 2;
}

<<__EntryPoint>>
function main() {
  $c = new C<int>();
  echo $c is C . "\n";
  $s = serialize($c);
  var_export($s);
  echo "\n";
  $c2 = unserialize($s);
  echo $c is C . "\n";
  echo $c is D<(int, int)> . "\n";
}
