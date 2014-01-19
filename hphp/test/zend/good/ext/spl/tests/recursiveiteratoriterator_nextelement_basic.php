<?php
$sample_array = array(1, 2, array(3, 4));
$sub_iterator = new RecursiveArrayIterator($sample_array);

$iterator = new RecursiveIteratorIterator($sub_iterator);
foreach ($iterator as $element) {
  var_dump($element);
}

class NextElementRecursiveIteratorIterator extends RecursiveIteratorIterator {
  public function nextElement() {
    echo "::nextElement() was invoked\n";
  }
}
$iterator = new NextElementRecursiveIteratorIterator($sub_iterator);
foreach ($iterator as $element) {
  var_dump($element);
}
?>