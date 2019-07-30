<?hh

class C<reify T> {}
class D {}

type Tx = int;
type Ty = classname<D>;
type Tz = typename<Tx>;

type Ta = C<classname<D>>;
type Tb = C<typename<Tx>>;
type Tc = C<Ty>; // TODO make reified check a type validator
type Td = C<Tz>; // TODO Make reified check a type validator
