<?hh

class SomeclassWithConstant {
  const CONSTANT = 'woogity boogity';
}

// This isn't actually a possible scenario. This is simply to demonstrate
// that other kinds of resolution expressions still fail.
<<__Sealed(SomeclassWithConstant::CONSTANT)>>
class SomeSealedClass {}

