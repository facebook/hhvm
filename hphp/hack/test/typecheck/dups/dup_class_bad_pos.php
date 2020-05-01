////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A {
  abstract const type T as mixed;
}
abstract class C {
}
////file2.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
trait MyTrait {
  require extends A;
}
abstract class C {
  use MyTrait;
  const type T = int;
}
