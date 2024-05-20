<?hh

function expect_string(string $s): void {}
function expect_nstring(?string $s): void {}

class C {
  public string $s = "";
  public ?string $ns = null;

  public static string $ss = "";
  public static ?string $sns = null;
}

function error_cases(): void {
  expect_string(C::class);
  expect_nstring(C::class);
}

function skip_reified<reify T as C>(): void {
  expect_string(T::class);
}

function error_only_syntactic(): void {
  $c = C::class;
  expect_string($c);
}

function ret_string(): string { return C::class; }
function ret_nstring(): ?string { return C::class; }

function prop_string(C $c): void {
  $c->s = C::class;
  C::$ss = C::class;
}
function prop_nstring(C $c): void {
  $c->ns = C::class;
  C::$sns = C::class;
}
