<?hh // strict

class :foo {
  attribute
    Stringish xlink:href,
    float opacity;
}

<foo xlink:href="test" opacity={0.25} />;
