<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface MyInterface {
  <<__Enforceable>>
  abstract const type TType as shape(...);
}

abstract class MyClassBase {
  const type TType = shape('foo' => vec<string>);
}

final class MyClass extends MyClassBase implements MyInterface {
}
