<?hh

function main() {
  $count = apc_fetch('rec-test-count');
  if ($count === false) $count = 0;
  if ($count >= 2) return;

  if ($count == 0)
    require 'record_decl.1.inc';
  else
    require 'record_decl.2.inc';

  if ($count == 0) {
    $x = Foo['x' => 42];
    $y = $x['x'];
  } else {
    $x = Foo['y' => 'hello'];
    $y = $x['y'];
  }
  var_dump($y);

  $count++;
  apc_store('rec-test-count', $count);
}

<<__EntryPoint>>
  function main_multi_record() {
    main();
  }
