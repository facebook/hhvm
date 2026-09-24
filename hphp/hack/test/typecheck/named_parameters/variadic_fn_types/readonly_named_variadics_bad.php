<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>
//
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

class ReadonlyNamedHead {
  public function __construct(named readonly string $head, named string...) {}
}

function test_readonly_declared_named_param_bad(readonly string $ro): void {
  new ReadonlyNamedHead(head = $ro, extra = $ro);
  // Repeating a declared name must not bind it to the mutable variadic.
  new ReadonlyNamedHead(head = $ro, head = $ro);
}
