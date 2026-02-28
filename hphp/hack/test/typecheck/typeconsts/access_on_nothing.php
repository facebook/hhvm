<?hh

class Cm {
  const type T = mixed;
}

function f0<T as Cm with { type T = Tm }, Tm>(mixed $x): Tm {
  return $x;
}

function spoof(mixed $x): nothing {
  return f0<nothing, _>($x);
}
