<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Base {}
class GenericErased<T> extends Base {}
class GenericReified<reify T> extends Base {}
class Concrete extends Base {}

abstract class Top {
  <<__Enforceable>>
  abstract const type TVassil as Base;
}

class E extends Top {
  const type TVassil = GenericErased<int>;
}

class R extends Top {
  const type TVassil = GenericReified<int>;
}

class C extends Top {
  const type TVassil = Concrete;
}
