<?hh

class A { const ctx C = []; }

function f<T>(
  A $a,
  (function (T)[_]: void) $f,
)[$a::C, ctx $f]: void {
  f<T>($a, $f);

  f<T, _, _>($a, $f);

  f<
    T,
    _, // T/$a
    _, // T/[$a::C]
    _, // T/[ctx $f]
  >($a, $f);

  f<T, _, _, _, _>($a, $f);
}
