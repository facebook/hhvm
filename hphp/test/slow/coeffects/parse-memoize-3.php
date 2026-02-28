<?hh

class C {
  <<__Memoize>>
  function f()[this::C] :mixed{}
}
