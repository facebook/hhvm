<?hh
class IA implements IteratorAggregate {
  public function getIterator():mixed{}
}

class Fatal extends IA implements Iterator {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
