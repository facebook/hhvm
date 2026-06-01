<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function takes_representable_as_null(\HH\Runtime\RepresentableAs<null> $c): void {}
function takes_representable_as_nonnull(\HH\Runtime\RepresentableAs<nonnull> $c): void {}

function returns_void(): void {}

function test_void_sub_representable_as_null(): void {
  // void <: RepresentableAs<null> — should pass
  takes_representable_as_null(returns_void());
}

function test_void_sub_representable_as_nonnull(): void {
  // void <: RepresentableAs<nonnull> — should FAIL
  takes_representable_as_nonnull(returns_void());
}
