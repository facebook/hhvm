<?hh

class C {
  <<__RequirePackage(foo, 123)>>
  function f(): void {}
  <<__RequirePackage(foo)>>
  function g(): void {}
}
