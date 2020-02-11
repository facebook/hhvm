<?hh

class NextElementRecursiveIteratorIterator extends RecursiveIteratorIterator {
  public function nextElement() {
    echo "::nextElement() was invoked\n";
  }
}
<<__EntryPoint>>
function main_entry(): void {
  $sample_array = varray[1, 2, varray[3, 4]];
  $sub_iterator = new RecursiveArrayIterator($sample_array);

  $iterator = new RecursiveIteratorIterator($sub_iterator);
  foreach ($iterator as $element) {
    var_dump($element);
  }
  $iterator = new NextElementRecursiveIteratorIterator($sub_iterator);
  foreach ($iterator as $element) {
    var_dump($element);
  }
}
