<?hh

function f() {
  $a = <<__Memoize>> async {
    return 1;
  };
}

