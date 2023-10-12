<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Contra<-T> { }
class Co<+T> { }

function foo1<T>(Co<T> $x, Contra<?T> $y):void { }

function foo2<T>(Contra<?T> $x, Co<T> $y): void { }

function test(Co<?string> $m, Contra<?string> $c): void {
  // This works
  foo1($m, $c);

  // This is accepted, with explicit type argument
  foo2<?string>($c, $m);

  // This should also be accepted
  // The only difference from foo1 is the parameter order
  foo2($c, $m);
}
