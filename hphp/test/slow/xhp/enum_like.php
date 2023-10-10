<?hh

class :foo {
  attribute
    ~enum {
      'foo',
      'bar',
      'baz',
    } e;
}


<<__EntryPoint>>
function main() :mixed{
  $x = <foo e={'bar'} />;
  echo "Done";
}
