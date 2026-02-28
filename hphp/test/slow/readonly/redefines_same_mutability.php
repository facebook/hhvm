<?hh

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() :mixed{
  $p = readonly new P(1);
  $p = readonly new P(2);
  $x = new P(3);
  $x = new P(4);
  echo "Done\n";
}
