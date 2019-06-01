<?hh

class C<reify T> {
  private int $x = 1;
  private int $y = 2;
}

<<__EntryPoint>>
function main() {
  $c = new C<int>();
  echo $c is C<int> . "\n";
  $s = serialize($c);
  var_export($s);
  echo "\n";
  $c2 = unserialize($s);
  echo $c is C<int> . "\n";
}
