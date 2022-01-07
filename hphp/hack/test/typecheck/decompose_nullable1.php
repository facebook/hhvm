<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

// All test cases in this file should produce an error due to the type being
// nullable

class A { public int $i = 0; }
class B { public int $i = 0; }
class C { public int $i = 0; }
class D { public int $i = 0; }

function test1(?A  $x): void { $x->i; }
function test2(?B  $x): void { $x->i; }
function test3((?A & ?B) $x): void { $x->i; }
function test4((?A & (C | ?B)) $x): void { $x->i; }
function test5((?A & (C | (B | ?A))) $x ): void { $x->i; }
function test6((?A & ?(C | ?A)) $x): void { $x->i; }
