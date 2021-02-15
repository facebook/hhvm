<?hh // strict

class :foo extends XHPTest {
  attribute int bar @required;
}

function testRequiredMakesNotNullable(): void {
  <foo bar={null} />;
}
