<?hh

enum Ei : int {
  Ki = 1;
}

enum Es : string {
  Ks = "1";
}

function expect_int(int $i) : void {}

function expect_string(string $i) : void {}

function expect_Es(Es $es) : void {}

function expect_Ei(Ei $ei) : void {}

function f(Es $es, Ei $ei, arraykey $ak) : void {
  if ($es is int) {
    expect_int($es);
    expect_Es($es);
  } else {
    expect_string($es);
    expect_Es($es);
  }

  if ($ei is int) {
    expect_int($ei);
    expect_Ei($ei);
  } else {
    expect_string($ei);
    expect_Ei($ei);
  }

  if ($ak is int) {
    expect_int($ak);
  } else {
    expect_string($ak);
  }
}

function g(Es $es, Ei $ei, arraykey $ak) : void {
  if ($es is string) {
    expect_string($es);
    expect_Es($es);
  } else {
    expect_int($es);
    expect_Es($es);
  }

  if ($ei is string) {
    expect_string($ei);
    expect_Ei($ei);
  } else {
    expect_int($ei);
    expect_Ei($ei);
  }

  if ($ak is string) {
    expect_string($ak);
  } else {
    expect_int($ak);
  }
}
