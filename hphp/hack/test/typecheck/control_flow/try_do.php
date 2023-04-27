<?hh

class A {}
class B {}

function test(): void {
  try {
    $x = 0;
    do {
      $x = "";
    } while (1 === 2);
  } finally {
    expect_string($x);
  }
}

function expect_string(string $_): void {}
