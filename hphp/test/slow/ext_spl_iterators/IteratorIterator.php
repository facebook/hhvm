<?hh

class A implements IteratorAggregate {
  function getIterator() {
    return new B;
  }
}
class B implements IteratorAggregate {
  function getIterator() {
    return new C;
  }
}
class C implements IteratorAggregate {
  function getIterator() {
    return new D;
  }
}
class D implements IteratorAggregate {
  function getIterator() {
    return new ArrayIterator(varray[1,2,3]);
  }
}


<<__EntryPoint>>
function main_iterator_iterator() {
foreach (new IteratorIterator(new A) as $v) {
  var_dump($v);
}
}
