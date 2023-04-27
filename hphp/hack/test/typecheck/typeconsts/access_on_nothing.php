<?hh

class Cm {
  const type T = mixed;
}

function f0<T as Cm, Tm>(mixed $x): Tm where Tm = T::T {
  return $x;
}

function spoof(mixed $x): nothing {
  return f0<nothing, _>($x);
}
