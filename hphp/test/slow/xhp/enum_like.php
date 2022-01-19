<?hh // strict

class :foo {
  attribute
    ~enum {
      'foo',
      'bar',
      'baz',
    } e;
}


<<__EntryPoint>>
function main() {
  $x = <foo e={'bar'} />;
  echo "Done";
}
