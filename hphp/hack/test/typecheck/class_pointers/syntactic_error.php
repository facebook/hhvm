<?hh
<<file:__EnableUnstableFeatures('class_const_default')>>
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

class Attr implements HH\FunctionAttribute {
  public function __construct(public string $s) {}
}

<<Attr("abcd".C::class)>>
function concat(): void {
  C::class."abcd";
  "abcd".C::class;
  $x = "abcd";
  $x .= C::class;
}

abstract class A {
  const string S = self::class;
  abstract const string T = self::class;
}
enum class EC: mixed {
  string A = E::class;
}

enum E: string {
  A = E::class;
}

function ak(dict<string, mixed> $d): void {
  $d[C::class];
  dict[C::class => 4];
  Map { C::class => 4 };
  keyset[C::class];
  Set { C::class };
  ImmSet { C::class };
}
