<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// CARE! the decl_from_stdin test looks at line 8 column 9 of this file

final class C2 {
  public function m2(): void {
    new C2();
  }
}
