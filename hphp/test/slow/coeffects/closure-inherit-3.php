<?hh

function defaults($f = null) :mixed{ if ($f) $f(); }
function rx($f = null)[rx]   :mixed{ if ($f) $f(); }
function pure($f = null)[]   :mixed{ if ($f) $f(); }

class A { const ctx C = [rx]; }

function foo(A $x)[$x::C] :mixed{
  defaults(() ==> { return defaults(); });
  rx(() ==> { return defaults(); });
  pure(() ==> { return defaults(); });
}

function bar(A $x)[$x::C, rx_local] :mixed{
  defaults(() ==> { return defaults(); });
  rx(() ==> { return defaults(); });
  pure(() ==> { return defaults(); });
}

<<__EntryPoint>>
function main() :mixed{
  foo(new A());
  bar(new A());
}
