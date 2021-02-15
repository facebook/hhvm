<?hh // strict

class :foo extends XHPTest {
  attribute
    Stringish xlink:href,
    float opacity;
}

function testAttributeWithColon(): void {
  <foo xlink:href="foo" opacity={0.25} />;
}
