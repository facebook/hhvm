<?hh // strict

class :foo {
  attribute
    Stringish xlink:href,
    float opacity;
}

function testAttributeWithColon(): void {
  <foo xlink:href="foo" opacity={0.25} />;
}
