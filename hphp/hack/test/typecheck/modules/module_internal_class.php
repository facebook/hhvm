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
  public function bar():void { }
}

interface I { }

class D
// Good
extends C
// Good
implements I {
  // Bad
  public ?C $bad1;
  // Good
  internal ?C $good1;
  // Bad
  public function bad2(C $x):void { }
  // Good
  internal function good2(C $c):void { }
  // Bad
  public function bad3():C { return new C(); }
  // Goood
  internal function good3():C { return new C(); }
}

internal class E extends C {} // Ok!


//// there.php
<?hh


module there;

class F extends C {} // Bad!


//// everywhere.php
<?hh

class G extends C {} // Bad!
