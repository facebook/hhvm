<?hh // strict

class :foo {
  attribute int bar = 123;
}

function testDefaultMakesNotNullable(): void {
  <foo bar={null} />;
}
