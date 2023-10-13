<?hh

newtype Foo<T> as string = string;

class C {
  private ?Foo $foo;
}
