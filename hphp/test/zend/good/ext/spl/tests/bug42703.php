<?hh
class BlaIterator implements Iterator
{
    public function rewind() :mixed{ }

    public function next() :mixed{ }

    public function valid() :mixed{
        return true;
    }

    public function current()
:mixed    {
      throw new Exception('boo');
    }

    public function key() :mixed{ }
}
<<__EntryPoint>> function main(): void {
$it = new BlaIterator();
$itit = new IteratorIterator($it);

try {
  foreach($itit as $key => $value) {
      echo $key, $value;
  }
}
catch (Exception $e) {
    var_dump($e->getMessage());
}

var_dump($itit->current());
var_dump($itit->key());
}
