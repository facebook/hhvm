<?hh // strict

class :foo {
  attribute
    enum {
      'foo',
      'bar',
      'baz',
    } e;
}


<<__EntryPoint>>
function main_enum_trailing_comma() {
echo 'Done';
}
