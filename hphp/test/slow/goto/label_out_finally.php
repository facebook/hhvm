<?hh
function foo($a) {
  try {
    if ($a < 5) {
      echo "Foo";
    }
    else {
      test:
        echo "bar";
    }
  } finally {
    goto test;
  }
}

<<__EntryPoint>>
function main_label_out_finally() {
foo(5);
echo "Done\n";
}
