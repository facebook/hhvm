<?hh

class C {
  <<__RequirePackage("foo")>>
  function f(): void {}
}

<<__RequirePackage("bar")>>
function h(): void {}
