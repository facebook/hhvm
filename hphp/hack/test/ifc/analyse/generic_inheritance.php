<?hh // strict

class A<+T2,-T3> { }
class B<T1,+T2,-T3> extends A<T2,T3> { }

function testAA(A<string,bool> $a): A<string,bool> { return $a; }
function testBB(B<int,string,bool> $b): B<int,string,bool> { return $b; }

// Nested instantiation
class C<T> {}
class D extends C<C<int>> {}
function nested(D $d): D { return $d; }

// Indirect inheritance of type parameters work as expected.
class E extends C<int> {}
class F extends E {}
function indirect(F $f): F { return $f; }

// Interfaces also work
interface I<-T> {}
class G implements I<int> {}
function interface_(G $g): G { return $g; }

// The `I<int>` implementation should be picked over `I<vec<int>>`
class H implements I<int> {}
class K extends H implements I<vec<int>> {}
function interfaceResolution(K $k): K { return $k; }

// Traits
trait T<+T> {}
class L { use T<int>; }
function trait_(L $l): L { return $l; }
