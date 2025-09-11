<?hh

class C {
  <<__RequirePackage(foo, 123)>>
  function f(): void {}
}
