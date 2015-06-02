<?hh // strict

class :foo {
  attribute int bar @required;
}

function testRequiredMakesNotNullable(): void {
  <foo bar={null} />;
}
