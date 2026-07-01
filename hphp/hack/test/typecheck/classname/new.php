<?hh

interface I {
  public function __construct();
}

<<__ConsistentConstruct>>
abstract class C {}

<<__ConsistentConstruct>>
abstract class D<Td> {}

<<__ConsistentConstruct>>
trait MyTr {}

function factory_i<T as I>(class<T> $klass): I {
  $inst = new $klass();
  hh_show($inst);
  return $inst;
}

function factory_c<T as C>(class<T> $klass): C {
  $inst = new $klass();
  hh_show($inst);
  return $inst;
}

function factory_d<Td as D<I>>(class<Td> $klass): Td {
  $inst = new $klass();
  hh_show($inst);
  return $inst;
}

// NOTE: this doesn't not work because of the current rules on typehints
// for uninstantiable constructs (abstract-final and traits)
// function factory_trait<T as MyTr>(class<T> $klass): T {
//   $inst = new $klass();
//   hh_show($inst);
//   return $inst;
// }

class Factory<T as C> {
  public function __construct(private class<T> $klass) {}
  public function make(): T {
    $klass = $this->klass;
    $inst = new $klass();
    hh_show($inst);
    return $inst;
  }
}
