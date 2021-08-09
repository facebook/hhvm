<?hh

class foo {
  const ctx Bar = [write_props];
}

function main()[foo::Bar]: void {
  echo "fail"; // error
}
