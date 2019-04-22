<?hh

function none() {}
set_error_handler('none');

class Foo {
}

function asdf($pattern) {
  $len = strlen($pattern);
  $len--;
  echo $len;
  echo "\n";
}

function main() {
  $foo = new Foo();
  for ($i=0; $i<100; $i++) {
    try {
      asdf(new Foo());
    } catch (Exception $e) {}
    try {
      asdf(array());
    } catch (Exception $e) {}
  }
}
main();
main();

function main2() {
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
main2();

echo "done\n";
