<?hh

class C1 {}
class C2 extends C1 {}
class C3 extends C2 {}

class BaseClass<+Tv> {}

interface I1 {
  require extends BaseClass<C1>;
}
interface I2 extends I1 {
  require extends BaseClass<C2>;
}
interface I3 extends I2 {
  require extends BaseClass<C3>;
}

class Test1 extends BaseClass<C2> implements
  I1,
  I2 {
}

class Test2 extends BaseClass<C3> implements
  I1,
  I2,
  I3 {
}
