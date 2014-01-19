<?php
$sample_array = array(1, 2);
$sub_iterator = new RecursiveArrayIterator($sample_array);

$iterator = new RecursiveIteratorIterator($sub_iterator);
foreach ($iterator as $element) {
  var_dump($element);
}

class SkipsFirstElementRecursiveIteratorIterator extends RecursiveIteratorIterator {
  public function beginIteration() {
    echo "::beginIteration() was invoked\n";
    $this->next();
  }
}
$iterator = new SkipsFirstElementRecursiveIteratorIterator($sub_iterator);
foreach ($iterator as $element) {
  var_dump($element);
}
?>