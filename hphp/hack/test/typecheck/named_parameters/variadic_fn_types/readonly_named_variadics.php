<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function readonly_head(readonly string $head, named string...): void {}
function mutable_tails(string ...$positional, named string...): void {}

function test_readonly_named_variadics(readonly string $ro): void {
  readonly_head($ro, extra = "ok");
  readonly_head(extra = "ok", $ro);
  mutable_tails("a", first = "b", "c", second = "d");
}
