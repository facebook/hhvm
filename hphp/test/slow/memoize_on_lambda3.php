<?hh

function f() {
  $a = <<__Memoize>> async {
    return 1;
  };
}

<<__EntryPoint>> function main(): void {}
