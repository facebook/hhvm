<?hh

<<file:__EnableUnstableFeatures('package'),  __PackageOverride('foo')>>

function test(): void {
  $_ = package foo;
}
