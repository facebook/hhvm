<?hh

class EndIterationRecursiveIteratorIterator extends RecursiveIteratorIterator {
  public function endIteration() {
    echo "::endIteration() was invoked\n";
  }
}
<<__EntryPoint>>
function main_entry(): void {
  $sample_array = varray[1, 2];
  $sub_iterator = new RecursiveArrayIterator($sample_array);

  $iterator = new RecursiveIteratorIterator($sub_iterator);
  foreach ($iterator as $element) {
    var_dump($element);
  }
  $iterator = new EndIterationRecursiveIteratorIterator($sub_iterator);
  foreach ($iterator as $element) {
    var_dump($element);
  }
}
