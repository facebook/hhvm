<?hh

class C = D;

class C<T> = D<T>;

interface I = J;

interface I<T> = J<T>;

// syntax error
trait T1 = T2;
