<?hh // strict

class A {}

function test(?A $a): void {
    if ($a === null) {
      expect_nullable_int($a);
      expect_nullable_A($a);
    } else {
      expect_A($a);
    }
    expect_nullable_A($a);
}

function expect_nullable_int(?int $a): void {}
function expect_A(A $a): void {}
function expect_nullable_A(?A $a): void {}
