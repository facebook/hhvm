<?hh

class rii extends RecursiveIteratorIterator
{
  function rewind() {
    echo __METHOD__ . "()\n";
    parent::rewind();
  }

  function beginIteration()
  {
    echo __METHOD__ . "()\n";
  }

  function endIteration()
  {
    echo __METHOD__ . "()\n";
  }

  function valid() {
    echo __METHOD__ . "()\n";
    return parent::valid();
  }
}

class MyAggregate implements IteratorAggregate {
  public function __construct(public $a) {}

  public function getIterator() {
    return new RecursiveArrayIterator($this->a);
  }
}

<<__EntryPoint>>
function main_recursive_iterator_iterator_begin_end_iteration() {
$ar = varray[1, 2, varray[31]];

$it = new rii(new MyAggregate($ar));

$it->rewind();
$it->rewind();
$it->rewind();

while($it->valid()) // call EndIteration on last call
  $it->next();

$it->valid(); // Don't call EndIteration
$it->valid(); // Don't
}
