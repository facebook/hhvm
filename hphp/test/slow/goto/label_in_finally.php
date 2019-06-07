<?hh
function foo() {
  try {
  } finally {
    goto test;
    test:
      echo "blah\n";
  }
}

<<__EntryPoint>>
function main_label_in_finally() {
foo();
echo "Done\n";
}
