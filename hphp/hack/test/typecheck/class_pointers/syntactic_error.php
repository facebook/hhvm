<?hh
<<file:__EnableUnstableFeatures('class_const_default')>>
function expect_string(string $s): void {}
function expect_nstring(?string $s): void {}
function expect_arraykey(arraykey $key): void {}
function expect_like_arraykey(~arraykey $key): void {}

class C {
  const classname<mixed> NAME = C::class;
  const arraykey KEY = C::class;

  public string $s = "";
  public ?string $ns = null;
  public arraykey $key = 0;

  public static string $ss = "";
  public static ?string $sns = null;
}

function error_cases(): void {
  expect_string(C::class);
  expect_nstring(C::class);
  expect_arraykey(C::class);
  expect_like_arraykey(C::class);
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
function ret_narraykey(): ?arraykey { return C::class; }

function prop_string(C $c): void {
  $c->s = C::class;
  C::$ss = C::class;
}
function prop_nstring(C $c): void {
  $c->ns = C::class;
  C::$sns = C::class;
}
function prop_arraykey(C $c): void {
  $c->key = C::class;
}

function explicit_names(class<C> $c): void {
  expect_arraykey(nameof C);
  expect_arraykey(123);
  expect_arraykey(HH\class_to_classname($c));
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
  abstract const arraykey KEY = self::class;
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
