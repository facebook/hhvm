<?hh

<<file:__EnableUnstableFeatures('package')>>

class C {
  <<__CrossPackage(foo, 123)>>
  function f(): void {}
}
