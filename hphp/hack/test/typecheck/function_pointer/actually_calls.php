<?hh

// The parsing changes for function pointers allows for a new syntax for calling
// functions and methods with an empty type argument set
// i.e. my_function_call<>($args)

function my_test(vec<int> $x): void {}

final class MyTest {
  public function foo(string $x): void {}
}

function main(?MyTest $z): void {
  my_test<>(vec[1,2,3]);

  $y = new MyTest();

  $y->foo<>("Hello");

  // Calls the method 'foo' if $z is not null
  $z?->foo<>("Hello");

  // Always tries to call the method 'foo', so this is a type error
  // because $z can be null
  ($z?->foo<>)("Hello");
}
