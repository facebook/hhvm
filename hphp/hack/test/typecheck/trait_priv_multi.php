////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T1 {
  private function f(): int { return 3; }
}
////file2.php
<?hh
class B {
  public function f(): int { return 4; }
}
////file3.php
<?hh
class C extends B {
  use T1;
}
