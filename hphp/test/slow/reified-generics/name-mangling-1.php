<?hh

newtype FBID = int;

class A<reify T> {
  public function getTypeStructure(): TypeStructure<T> {
    return HH\ReifiedGenerics\get_type_structure<T>();
  }
}
final class B extends A<FBID> {}
final class C extends A<int> {}

<<__EntryPoint>>
function main() :mixed{
  $b = new B();
  $c = new C();

  var_dump($b->getTypeStructure());
  var_dump($c->getTypeStructure());
}

