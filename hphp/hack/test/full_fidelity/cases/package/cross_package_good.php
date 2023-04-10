<?hh

<<file:__EnableUnstableFeatures('package')>>

class C {
  <<__CrossPackage("foo")>>
  function f(): void {}

  <<__CrossPackage("foo", "bar")>>
  function g(): void {}
}

<<__CrossPackage("bar")>>
function h(): void {}
