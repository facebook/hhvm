<?hh

class C<reify T> {}
class D {}

type Tx = int;
type Ty = classname<D>;
type Tz = typename<Tx>;

type Ta = C<classname<D>>;
type Tb = C<typename<Tx>>;
type Tc = C<Ty>;
type Td = C<Tz>;
