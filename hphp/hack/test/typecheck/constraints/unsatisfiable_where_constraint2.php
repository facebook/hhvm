//// file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I<T> {
  public function f(): void where T = int;
}

trait S<T> implements I<T> {
  public function f(): void where T = int {}
}

//// file2.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C implements I<string> {
  use S<string>;
}
