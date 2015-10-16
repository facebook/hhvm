<?hh // strict

interface Foo {}

abstract class Bar {}

trait MyTrait<T as Foo> {
  require extends int;
}
