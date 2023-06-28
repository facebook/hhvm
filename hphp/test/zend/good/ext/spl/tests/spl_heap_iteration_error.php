<?hh
class ext_heap extends SplMaxHeap {
  public $fail = false;
  public function compare($val1,$val2) :mixed{
    if ($this->fail)
      throw new Exception('Corrupting heap',99);
    return 0;
  }
}
<<__EntryPoint>> function main(): void {
$h = new ext_heap();
$h->insert(varray['foobar']);
$h->insert(varray['foobar1']);
$h->insert(varray['foobar2']);

try {
  $h->fail=true;
  foreach ($h as $value) {};
  echo "I should have raised an exception here";
} catch (Exception $e) {
  if ($e->getCode()!=99) echo "Unexpected exception";
}

var_dump($h);
}
