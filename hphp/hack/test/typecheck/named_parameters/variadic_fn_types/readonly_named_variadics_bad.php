<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class NamedTail {
  public function __construct(named string...) {}
}

class MixedTails {
  public function __construct(string ...$positional, named string...) {}
}

function mixed_tails(string ...$positional, named string...): void {}

function test_readonly_named_variadics_bad(readonly string $ro): void {
  new NamedTail(first = $ro);
  new NamedTail(first = "ok", second = $ro);

  mixed_tails(first = "ok", "a", "b", $ro);
  mixed_tails("a", "b", $ro, first = "ok");

  new MixedTails(first = "ok", second = $ro, "a");
  new MixedTails("a", first = "ok", second = $ro);
}
