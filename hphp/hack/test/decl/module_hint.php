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
    $m as D;
    $m as G<_>;
    $m as (D,int);
    $m is D;
    $m is G<_>;
    $m is (D,int);
    $m as E;
    $m is E;
    genfun<D>(null);
    $x = vec<D>[];
    $x = new MyList<D>();
    // All ok
    $m as C;
    $m is C;
  }
}

//// there.php
<?hh

module there;

internal class D {
}
internal class G<T> {
}
internal enum E : int {
  A = 1;
}

//// everywhere.php
<?hh

function genfun<T>(?T $x):?T { return $x; }

class MyList<T> { }
