<?hh

interface Foo {}

trait MyTrait<T as Foo> {
  require implements T;
}
