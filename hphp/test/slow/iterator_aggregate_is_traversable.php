<?hh

class MyIA implements IteratorAggregate {
  public function getIterator() :mixed{
    return Vector{1, 2};
  }
}

<<__EntryPoint>> function main() :mixed{
  $ia = new MyIA();
  var_dump($ia is \HH\Traversable);
  var_dump($ia is Traversable); // same as \HH\Traversable due to auto-import
  foreach ($ia as $v) {
    var_dump($v);
  }
  echo "Done\n";
}
