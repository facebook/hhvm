<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class C {
  abstract const type TX;

  abstract public function gen(): this::TX;
  abstract public function log(this::TX $x): void;
}

function my_gen<T as C, TR>(T $left) : TR where TR = T::TX {
  return $left->gen();
}

  function my_log<T as C, TR>(T $left) : (function (TR): void ) where TR = T::TX {
  return (TR $x) ==> ($left->log($x));
}

  function id(C $x) : C { return $x; }
// Correctly generates an error
/*
function my_error(C $x, C $y) : void {
  $gen = my_gen($x);
  $log = my_log($y);
  $log($gen);
}
*/
// Do not raise any error !
function my_type_hole(C $x, C $y) : void {
  $gen = my_gen(id($x));
  $log = my_log(id($y));
  $log($gen);
}

class D extends C {
  const type TX = int;
  public function gen():int { return 3; }
  public function log(int $_):void { }
}
class E extends C {
  const type TX = string;
  public function gen():string { return "A"; }
  public function log(string $_):void { }
}
<<__EntryPoint>>
function main():void {
  my_type_hole(new D(), new E());
}
