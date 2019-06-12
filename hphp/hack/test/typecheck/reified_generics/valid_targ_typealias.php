<?hh // partial

class C<reify T> {}
newtype Opaque = int;
type Transparent = int;

type Ty1 = C<Opaque>;
type Ty2 = C<Transparent>;
