<?hh
class IA implements IteratorAggregate {
  public function getIterator(){}
}

class Fatal extends IA implements Iterator {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
