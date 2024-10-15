<?hh

<<file:__EnableUnstableFeatures('require_package')>>

class C {
  <<__RequirePackage(foo, 123)>>
  function f(): void {}
}
