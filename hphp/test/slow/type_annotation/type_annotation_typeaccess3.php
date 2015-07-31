<?hh

class C {
  const SBAR = 'bar';
  const type TD = D;
  const type U = shape(
    'foo' => C::TD::TE::TF::T,
    C::SBAR => Vector<this::TD::T>,
  );
}

class D {
  const type TE = E;
  const type T = bool;
}

class E {
  const type TF = F;
}

class F {
  const type T = float;
}

var_dump(type_structure(C::class, 'U'));
