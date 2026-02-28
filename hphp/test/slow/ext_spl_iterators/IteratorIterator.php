<?hh

class A implements IteratorAggregate {
  function getIterator() :mixed{
    return new B;
  }
}
class B implements IteratorAggregate {
  function getIterator() :mixed{
    return new C;
  }
}
class C implements IteratorAggregate {
  function getIterator() :mixed{
    return new D;
  }
}
class D implements IteratorAggregate {
  function getIterator() :mixed{
    return new ArrayIterator(vec[1,2,3]);
  }
}


<<__EntryPoint>>
function main_iterator_iterator() :mixed{
foreach (new IteratorIterator(new A) as $v) {
  var_dump($v);
}
}
