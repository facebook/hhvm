<?PHP

class A implements IteratorAggregate {
  public function getIterator() {
    return new ArrayIterator(array(1,2,3));
  }
}

var_dump(iterator_to_array(new ArrayIterator(array(1,2,3))));
var_dump(iterator_to_array(new A));
var_dump(iterator_to_array(new stdClass));
