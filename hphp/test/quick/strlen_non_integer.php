<?php

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
    asdf(new Foo());
    asdf(array());
  }
}
main();
main();

function main2() {
  echo "foo: " .strlen($x)."\n";
  echo "foo: " .strlen(true)."\n";
  echo "foo: " .strlen(NULL)."\n";
  echo "foo: " .strlen(false)."\n";
}
main2();

echo "done\n";
