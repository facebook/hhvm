<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class A {}
function f(<<__Soft>> class<A> $a): void {}

<<__EntryPoint>>
function main(): void {
  f(false);
  f(3.14);
  f(3);
  f(vec[]);
  f(dict[]);
  f(new A());
}
