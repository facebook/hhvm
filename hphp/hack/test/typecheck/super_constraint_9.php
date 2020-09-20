<?hh // partial

// Notice that foo is invariant in T
class Foo<T> {}

/* HH_FIXME[4110] */
function f<Tk1, Tk2 super Tk1>(Foo<Tk1> $x, Foo<Tk2> $y): Foo<Tk2> {
}

function g(Foo<string> $x, Foo<int> $y): Foo<mixed> {
  // This should fail to check because Tk1=string and Tk2=int
  return f($x, $y);
}

/* HH_FIXME[4110] */
function bar(): Foo<int> {
}

function h(Foo<string> $x): Foo<mixed> {
  // This should fail to check because Tk1=string and Tk2=int
  return f($x, bar());
}
