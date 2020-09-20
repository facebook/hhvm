<?hh

class C<reify T> {}
class D {}
class E<T> {}

type Tx = int;
type Ty = classname<D>;
type Tz = typename<Tx>;

type Ta = C<classname<D>>;
type Tb = C<typename<Tx>>;
type Tc = C<Ty>;
type Td = C<Tz>;
type Te = C<E<classname<D>>>;
type Tf = C<E<typename<Tx>>>;
