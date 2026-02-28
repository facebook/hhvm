////file1.php
<?hh

function any() /* : Tany */ { return 3; }

////file2.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

  function testit(num $n, arraykey $ak, bool $b, dynamic $d):void {
  $x1 = $n . $b;
  $x2 = $n . $ak;
  $x3 = $ak . $b;
  $x4 = null . $ak;
  $x5 = null . $n;
  $x6 = null . $b;
  $x7 = $n . any();
  $x8 = $ak . $d;
}
