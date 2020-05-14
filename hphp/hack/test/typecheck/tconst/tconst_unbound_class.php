////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  const type T = D::T;
}

////file2.php
<?hh
function foo(C::T $x):void { }
