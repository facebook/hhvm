<?hh

class D<reify T> {
  private int $z = 1;
}

class C<reify T> extends D<(T, T)> {
  private int $x = 1;
  private int $y = 2;
}

<<__EntryPoint>>
function main() :mixed{
  $c = new C<int>();
  echo (string)($c is C<int>) . "\n";
  $s = serialize($c);
  var_export($s);
  echo "\n";
  $c2 = unserialize($s);
  echo (string)($c is C<int>) . "\n";
  echo (string)($c is D<(int, int)>) . "\n";
}
