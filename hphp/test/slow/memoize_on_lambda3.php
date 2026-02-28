<?hh

function f() :mixed{
  $a = <<__Memoize>> async {
    return 1;
  };
}

