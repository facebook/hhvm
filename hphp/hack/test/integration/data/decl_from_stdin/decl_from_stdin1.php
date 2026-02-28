<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// CARE! the decl_from_stdin test looks at line 13 column 9 of this file

class C1 {
  public function m1(): void {}
}

final class D1 {
  public function n1(): void {
    $x = new C1();
    $x->m1();
  }
}
