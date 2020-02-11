<?hh

class SkipsFirstElementRecursiveIteratorIterator extends RecursiveIteratorIterator {
  public function beginIteration() {
    echo "::beginIteration() was invoked\n";
    $this->next();
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
  $iterator = new SkipsFirstElementRecursiveIteratorIterator($sub_iterator);
  foreach ($iterator as $element) {
    var_dump($element);
  }
}
