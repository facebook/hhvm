<?hh // partial

class C<reify T> {}

type Ty2 = C<varray<int>>;
type Ty3 = C<darray<int, string>>;
type Ty4 = C<varray_or_darray<string>>;
