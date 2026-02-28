<?hh

function alpha() :mixed{
  throw new Error('Foo');
}

function beta() :mixed{
  alpha();
}

function gamma() :mixed{
  beta();
}

function main() :mixed{
  try {
    gamma();
  } catch (Throwable $t) {
    var_dump($t->getMessage());
  }
}


<<__EntryPoint>>
function main_error_002() :mixed{
main();
}
