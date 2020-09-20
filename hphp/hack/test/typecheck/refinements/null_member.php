<?hh // strict

class A {}

class Test {
  public ?A $a;

  public function test1(): void {
    if ($this->a === null) {
      expect_nullable_int($this->a);
      expect_nullable_A($this->a);
    } else {
      expect_A($this->a);
    }
    expect_nullable_A($this->a);
  }

  public function test2(): void {
    if ($this->a !== null) {
      expect_A($this->a);
    } else {
      expect_nullable_int($this->a);
      expect_nullable_A($this->a);
    }
    expect_nullable_A($this->a);
  }
}

function expect_nullable_int(?int $a): void {}
function expect_A(A $a): void {}
function expect_nullable_A(?A $a): void {}
