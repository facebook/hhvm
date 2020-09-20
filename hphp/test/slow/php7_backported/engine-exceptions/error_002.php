<?hh

function alpha() {
  throw new Error('Foo');
}

function beta() {
  alpha();
}

function gamma() {
  beta();
}

function main() {
  try {
    gamma();
  } catch (Throwable $t) {
    var_dump($t->getMessage());
  }
}


<<__EntryPoint>>
function main_error_002() {
main();
}
