<?hh

function foobar(mixed $foo): void {
  if (is_array($foo)) {
    hh_show($foo);
    $_ = vec($foo);
    $_ = $foo[42];
    $_ = $foo['foo'];
    takes_container($foo);
    takes_vec($foo);
  }
}

function takes_container(Container<mixed> $_): void {}
function takes_vec(vec<mixed> $_): void {}
