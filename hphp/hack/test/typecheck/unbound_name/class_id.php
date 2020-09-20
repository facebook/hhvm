<?hh

final class Foo {
  public static int $baz = 4;

  const string BAR = "bar";

  public function quz(): void {}

  public static function raz(): void {}
}

type Alias = Foo;

function test_class_const<reify T as Foo>(): void {
  Foo::BAR;
  Foo::raz();

  // Unbound
  NotFound::BAR;
  NotFound::raz();

  // Allow generics
  T::BAR;
  T::raz();

  // Do not allow typedefs
  Alias::BAR;
  Alias::raz();
}

function test_class_get<reify T>(): void {
  Foo::$baz;

  // Unbound
  NotFound::$baz;

  // Do not allow typedefs
  Alias::$baz;
}

function test_new<<<__Newable>> reify T as Foo>(): void {
  new Foo();

  // Unbound
  new NotFound();

  // Allow generics
  new T();

  // Do not allow typedefs
  new Alias();
}
