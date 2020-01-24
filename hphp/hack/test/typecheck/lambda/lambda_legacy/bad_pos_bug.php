////file1.php
<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I { }
class C { }
interface J { }
 function foo(
 ): void {
   $p = (/*I*/ $_vc, C $x) ==> { };
   $ret = Map {
      'A' =>
        bar(dict[ 'B' => 0]),
      'C' => $p,
    };
  }

////file2.php
<?hh
function bar
  <T2 as I, T as J>(
  dict<string,mixed> $a
): (function(T2, T): Awaitable<void>){
  throw new Exception("A");
}
