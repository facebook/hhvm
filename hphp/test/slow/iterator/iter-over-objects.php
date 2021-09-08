<?hh

class MyIterator implements Iterator {
  private $position = 0;
  private $array = vec['a', 'b'];

  public function __construct() {
    $this->position = 0;
  }

  public function rewind() {
    $this->position = 0;
  }

  public function current() {
    return $this->array[$this->position];
  }

  public function key() {
    return $this->position;
  }

  public function next() {
    ++$this->position;
  }

  public function valid() {
    return isset($this->array[$this->position]);
  }
}

function gen() {
  yield 'a';
  yield 'b';
}

async function gen_async() {
  yield 0 => 'a';
  yield 1 => 'b';
}

<<__EntryPoint>>
async function main() {
  foreach (new MyIterator() as $key => $value) echo $key . ' => ', $value . "\n";
  foreach (new MyIterator() as $key => $value) echo $key . ' => ', $value . "\n";
  foreach (gen() as $key => $value) echo $key . ' => ', $value . "\n";
  foreach (gen_async() await as $key => $value) echo $key . ' => ', $value . "\n";
}
