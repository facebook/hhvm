<?php
class ext_heap extends SplMaxHeap {
  public $fail = false;
  public function compare($val1,$val2) {
    if ($this->fail)
      throw new Exception('Corrupting heap',99);
    return 0;
  }
}

$h = new ext_heap();
$h->insert(array('foobar'));
$h->insert(array('foobar1'));
$h->insert(array('foobar2'));

try {
  $h->fail=true;
  foreach ($h as $value) {};
  echo "I should have raised an exception here";
} catch (Exception $e) {
  if ($e->getCode()!=99) echo "Unexpected exception";
}

var_dump($h);
?>