<?hh

function defaults($f = null) { if ($f) $f(); }
function rx($f = null)[rx]   { if ($f) $f(); }
function pure($f = null)[]   { if ($f) $f(); }

class A { const ctx C = [rx]; }

function foo(A $x)[$x::C] {
  defaults(() ==> { return defaults(); });
  rx(() ==> { return defaults(); });
  pure(() ==> { return defaults(); });
}

function bar(A $x)[$x::C, rx_local] {
  defaults(() ==> { return defaults(); });
  rx(() ==> { return defaults(); });
  pure(() ==> { return defaults(); });
}

<<__EntryPoint>>
function main() {
  foo(new A());
  bar(new A());
}
