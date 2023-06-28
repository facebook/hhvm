<?hh

function test1(... $args) :mixed{
    var_dump($args);
}

function test2($arg1, $arg2, ...$args) :mixed{
    var_dump($arg1, $arg2, $args);
}

<<__EntryPoint>>
function main_entry(): void {

  test1();
  test1(1);
  test1(1, 2, 3);

  test2(1, 2);
  test2(1, 2, 3);
  test2(1, 2, 3, 4, 5);
}
