<?hh

class C {
  <<__Memoize>>
  function f(mixed $x)[$x::C] :mixed{}
}
