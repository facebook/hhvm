<?hh
abstract class Foo {
  abstract function f1() : (function():void);
  abstract function f2() : (function(int):void);
  abstract function f3() : (function(int,string):void);
  abstract function f4() : (function(...):void);
  abstract function f5() : (function(int,...):void);
  abstract function f6() : (function(int,string,...):void);
  abstract function g1() : Foo<(function():void)>;
  abstract function g2() : Foo<(function(int):void)>;
  abstract function g3() : Foo<(function(int,string):void)>;
  abstract function g4() : Foo<(function(...):void)>;
  abstract function g5() : Foo<(function(int,...):void)>;
  abstract function g6() : Foo<(function(int,string,...):void)>;
  abstract function h1((function():void) $x) : void;
  abstract function h2((function(int):void) $x) : void;
  abstract function h3((function(int,string):void) $x) : void;
  abstract function h4((function(...):void) $x) : void;
  abstract function h5((function(int,...):void) $x) : void;
  abstract function h6((function(int,string,...):void) $x) : void;
  abstract function j1(Foo<(function():void)> $x) : void;
  abstract function j2(Foo<(function(int):void)> $x) : void;
  abstract function j3(Foo<(function(int,string):void)> $x) : void;
  abstract function j4(Foo<(function(...):void)> $x): void;
  abstract function j5(Foo<(function(int,...):void)> $x): void;
  abstract function j6(Foo<(function(int,string,...):void)> $x): void;
}
echo "Done\n";
