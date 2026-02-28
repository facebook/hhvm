////file1.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  <<__Enforceable>>
  abstract const type T as vec<arraykey>;
}

////file2.php
<?hh
abstract class D extends C {
  abstract const type T as vec<arraykey> = vec<int>;
}
////file3.php
<?hh

class F extends D {
}
