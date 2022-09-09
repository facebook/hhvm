<?hh

class MyClass {
  // Printing the NAST should not crash if we have a naming
  // error. Instead, we should just print it.
  const int X = baz();
}

function baz(): void {}
