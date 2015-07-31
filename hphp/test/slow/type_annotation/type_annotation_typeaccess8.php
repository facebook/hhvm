<?hh

trait MyTrait{
}

class C {
  const type T = MyTrait;
  const type U = C::T::T1;
}

var_dump(type_structure(C::class, 'U'));
