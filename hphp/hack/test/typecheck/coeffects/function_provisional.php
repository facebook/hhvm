<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function f()@{ int }: void {
  f(); // ok
  g(); // ok
}

function g()@{ arraykey }: void {
  f(); // error, arraykey </: int
  g();
}

function g_unsafe()@{ arraykey + int }: void {
  f(); // unsafely ok, (arraykey&int) <: int
  g();
}
