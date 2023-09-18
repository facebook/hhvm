//// module_here.php
<?hh
new module here {}

//// module_there.php
<?hh
new module there {}

//// here.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module here;

internal class C {
  public function bar(mixed $m):void {
    // All not ok
    $x1 = new D();
    $y1 = D::class;
    D::foo();
    // All ok
    $x2 = new C();
    $y2 = C::class;
    C::foo();
    $x3 = new self();
    $y3 = self::class;
    self::foo();
  }
  public static function foo():void { }
}

//// there.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module there;

internal class D {
  public static function foo():void { }
}
