//// file1.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T;
}

class B {
  const type T = vec<A::T>;

  public function get(): this::T {
    return vec[];
  }
}

//// file2.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C extends B {
  public function test(): void {
    // make sure the error about abstract A::T is not raised here.
    $x = $this->get();
  }
}
