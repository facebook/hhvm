//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module here {}
module there {}

//// here.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules'), __Module('here')>>

<<__Module('here'), __Internal>>
class C {
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
<<file:__EnableUnstableFeatures('modules'), __Module('there')>>

<<__Module('there'), __Internal>>
class D {
}
<<__Module('there'), __Internal>>
class G<T> {
}
<<__Module('there'), __Internal>>
enum E : int {
  A = 1;
}

//// everywhere.php
<?hh

function genfun<T>(?T $x):?T { return $x; }

class MyList<T> { }
