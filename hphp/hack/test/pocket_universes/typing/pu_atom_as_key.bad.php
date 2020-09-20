<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

function testit(): void {
  $shape = shape("toto" => 42);
  echo $shape[:@tata]."\n";
}

class C {
  enum E {
    :@A;
  }
}

type D = dict<C:@E, string>;

function testit_var(D $dict, string $key) : ?string {
  return $dict[$key] ?? null;
}

function testit_literal(D $dict) : void {
  $dict[:@A] = "foo";
  $foo = $dict["A"];
  echo $foo;
  echo "\n";
}
