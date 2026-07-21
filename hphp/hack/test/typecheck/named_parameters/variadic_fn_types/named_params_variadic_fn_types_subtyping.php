<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// Subtyping tests for function types with named variadic parameters.
//
// A named variadic parameter `named T...` accepts any number of named
// arguments of type T. For sub <: super between two function types:
//   - If super has `named T_super...`, sub must also have `named T_sub...`
//     with T_super <: T_sub (contravariant element type), because the
//     consumer of super may send arbitrary named args that sub must absorb.
//   - If sub has `named T_sub...`, sub's variadic can absorb any required
//     named param declared in super, with a contravariant element-type
//     check against each such name.
//   - Sub having a named variadic that super lacks is fine — sub is
//     merely more permissive.

// OK: identical named variadic.
function take1((function(named int...): void) $_): void {}
function f1(named int ...$xs): void {}
function test1(): void {
  take1(f1<>);
}

// OK: sub has named variadic; super has no named params. Consumer sends
// no named args; sub's variadic accepts zero.
function take2((function(): void) $_): void {}
function f2(named int ...$xs): void {}
function test2(): void {
  take2(f2<>);
}

// OK: sub has named variadic; super has concrete named params. Sub's
// variadic absorbs them, and the element type is contravariantly
// compatible.
function take3((function(named int $a, named int $b): void) $_): void {}
function f3(named int ...$xs): void {}
function test3(): void {
  take3(f3<>);
}

// OK: positional + named variadic on both sides.
function take4((function(int, named string...): void) $_): void {}
function f4(int $x, named string ...$rest): void {}
function test4(): void {
  take4(f4<>);
}

// OK: contravariance in the variadic element type — sub accepts a wider
// type than super promises.
function take5((function(named int...): void) $_): void {}
function f5(named arraykey ...$xs): void {}
function test5(): void {
  take5(f5<>);
}

// ERROR: sub is less permissive than super — sub requires `x` but
// super may pass any named arg. Consumer might send `y` (unexpected) or
// no args at all (missing `x`).
function take_err1((function(named int...): void) $_): void {}
function f_err1(named int $x): void {}
function test_err1(): void {
  take_err1(f_err1<>);
}

// ERROR: variadic element type is not contravariantly compatible — sub
// accepts named strings, super promises named ints, so a caller of the
// super type sending an int arg would fail.
function take_err2((function(named int...): void) $_): void {}
function f_err2(named string ...$xs): void {}
function test_err2(): void {
  take_err2(f_err2<>);
}

// ERROR: super declares a required named `x: int`, sub tries to absorb
// it via a variadic whose element type is string — int is not a subtype
// of string.
function take_err3((function(named int $x): void) $_): void {}
function f_err3(named string ...$xs): void {}
function test_err3(): void {
  take_err3(f_err3<>);
}
