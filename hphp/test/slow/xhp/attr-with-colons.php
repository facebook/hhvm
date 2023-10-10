<?hh

class :foo {
  attribute
    Stringish xlink:href,
    float opacity;
}


<<__EntryPoint>>
function main_attr_with_colons() :mixed{
<foo xlink:href="test" opacity={0.25} />;
}
