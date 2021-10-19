<?hh //strict

class P {
  public function __construct(public int $i) { }
}

<<__EntryPoint>>
function main() {
  $p = new P(1);
  $p = readonly new P(1);
  echo "Done\n";
}
