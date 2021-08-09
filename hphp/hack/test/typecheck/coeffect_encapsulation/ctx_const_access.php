<?hh

class Foo {
  const ctx Bar = [write_props];
}

function main()[Foo::Bar]: void {
  echo "fail"; // error
}
