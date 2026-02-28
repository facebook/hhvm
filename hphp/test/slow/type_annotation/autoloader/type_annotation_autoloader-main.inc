<?hh

class C {
  const type T = this;
}

// Test that we do not attempt to autoload a bultin class
type BuiltinClass = HH\KeyedContainer;

// Test that we do not attempt to autoload an already loaded class
type LoadedClass = C;

// Test that we autoload a class in another file
type AutoloadClass1 = TestAutoloadedClass1;

// Test that we don't autoload an already loaded class
type AutoloadClass2 = TestAutoloadedClass2;

// The same for aliases
type AutoloadType1 = TestAutoloadedType1;

type AutoloadType2 = TestAutoloadedType2;
