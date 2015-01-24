<?hh

interface I1 {
  const int C1 = 10;
}

interface I2 extends I1 {}

interface I3 {
  const string C1 = 'foo';
}

class A implements I2 {}

class B extends A implements I3 {}
