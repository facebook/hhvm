<?hh

class X<+T, -T2> {}
class X2<+T, -T2> {}
type Y<T1, -T2, +T3> = int;

// Questionable, dissallowed in the type-checker
function foo<+T>() {}
