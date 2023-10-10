<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static int $a = 5;
  <<__Const>>
  public static int $ca = 6;
}

<<__EntryPoint>>
function test() :mixed{
  mutate(inout A::$a);
  var_dump(A::$a);

  try {
    mutate(inout A::$ca);
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump(A::$ca);
}

function mutate(inout int $a): void{
  $a = 50;
}
