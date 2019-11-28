////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T1 {
  private function f(): int { return 3; }
}
////file2.php
<?hh
trait T2 {
  public function f(): int { return 4; }
}
////file3.php
<?hh
class C {
  use T1;
  use T2;
}
