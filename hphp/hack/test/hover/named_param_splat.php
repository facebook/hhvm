<?hh

class Splat<Targs as (mixed...)> { }

class C1 extends Splat<(string,int)> { }

function foo<Targs as (mixed...)>(Splat<Targs> $splat, ...Targs $args):void { }

function call_it(): void {
  foo(new C1, "stuff", 1);
  //          ^ hover-at-caret
}
