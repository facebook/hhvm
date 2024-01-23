<?hh

function none() :mixed{}


class Foo {
}

function asdf($pattern) :mixed{
  $len = strlen($pattern);
  $len--;
  echo $len;
  echo "\n";
}

function main() :mixed{
  $foo = new Foo();
  for ($i=0; $i<100; $i++) {
    try {
      asdf(new Foo());
    } catch (Exception $e) {}
    try {
      asdf(vec[]);
    } catch (Exception $e) {}
  }
}



function main2() :mixed{
  try {
    echo "foo: " .strlen($x)."\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "foo: " .strlen(true)."\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "foo: " .strlen(NULL)."\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "foo: " .strlen(false)."\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  main();
  main();
  main2();

  echo "done\n";
}
