<?hh

class C {
  <<__RequirePackage("foo")>>
  function f(): void {}

  <<__RequirePackage("foo", "bar")>>
  function g(): void {}
}

<<__RequirePackage("bar")>>
function h(): void {}
