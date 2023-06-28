<?hh

<<__EntryPoint>>
function main_closure_static() :mixed{
  $a = Map{};
  $foo = function () use($a) {
    if (!$a) $a['xxxxxxxxxxxxx'] = str_repeat('x', 2048);
    echo "hello\n";
  };
  $foo();
}
