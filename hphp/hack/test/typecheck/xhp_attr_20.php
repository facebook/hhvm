<?hh

class :foo extends XHPTest {
  attribute int bar = 123;
}

function testDefaultMakesNotNullable(): void {
  <foo bar={null} />;
}
