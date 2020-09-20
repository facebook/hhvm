<?hh // strict

function foo(?int $i): void {
  if ($i === null) {
    expect_optional_string($i);
  }
}

function expect_optional_string(?string $i): void {}
