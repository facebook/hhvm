<?hh

<<file:__EnableUnstableFeatures('package')>>

class C {
  <<__CrossPackage("foo")>>
  function f(): void {}
}

<<__CrossPackage("bar")>>
function h(): void {}
