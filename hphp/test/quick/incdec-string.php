<?hh

function handler() :mixed{}

function foo() :mixed{
  $go = function($start) {
    $x = $start;
    for ($i = 0; $i < 10000; ++$i) {
      ++$x;
      if (($i % 101) == 0) { var_dump($x); var_dump(strlen($x)); }
    }
  };
  $go('a');
  $go('A');
  $go('A1');
  $go('a1');
}

<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(handler<>);
  foo();
}
