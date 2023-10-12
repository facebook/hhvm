<?hh

/**
 * Test cases for trailing commas in (sometimes) unexpected places. Try to do
 * this with *and* without line breaks (where trialing commas may add value).
 */
class Plain {
}

class Parametric<T,> {
}

function foo(dict<int,Plain,> $x): dict<
  int,
  Plain,
> {
  return $x;
}

function bar(keyset<int,> $x): keyset<
  int,
> {
  return $x;
}

function baz(vec<Plain,> $x): vec<
  Plain,
> {
  return $x;
}

function qux(Parametric<T,> $x): Parametric<
  T,
> {
  return $x;
}

function quux<T>(classname<T,> $x): classname<
  T,
> {
  return $x;
}
