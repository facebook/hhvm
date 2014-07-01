<?hh

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

type Nus = int;
enum Bar : Nus {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

enum Baz : mixed {
  FOO = 1;
  BAR = "welp";
}


function test(): @Foo {
  return Foo::FOO;
}

function test2(@int $x): void {
  var_dump($x);
}

function lurr(): void {
  test2(Foo::BAR);
}

function do_case(@Bar $x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
    case Bar::BAR:
      return 1;
    case Bar::BAZ:
      return 2;
  }
  return -1;
}

function welp(@Baz $x): void {
  var_dump($x);
}

var_dump(test());
lurr();
var_dump(do_case(Bar::BAR));
// This is bogus and should produce warning
var_dump(do_case("welp"));
welp(Baz::FOO);
welp(Baz::BAR);
// This is bogus but will work fine
welp(20);
// This is bogus and should produce warning
welp(20.0);

//
