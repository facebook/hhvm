<?hh

class A implements IteratorAggregate {
  public function getIterator() :mixed{
    return new ArrayIterator(vec[1,2,3]);
  }
}
<<__EntryPoint>> function main(): void {
var_dump(iterator_to_array(new ArrayIterator(vec[1,2,3])));
var_dump(iterator_to_array(new A));
var_dump(iterator_to_array(new stdClass));
}
