<?hh

function fail($t, $n, $e) {
  var_dump($t, $n);
  if ($e is Exception) {
    var_dump($e->getMessage(), $e->getTrace());
  } else {
    var_dump($e);
  }
  return false; // stop: otherwise hoistability would create an infinite loop
}

<<__EntryPoint>>
function entrypoint_autoloadfailhoist(): void {

  \HH\autoload_set_paths(
    darray['class' => darray[//
            'i1' => 'autoload-fail-e.inc',
            'i2' => 'autoload-fail-e.inc',
          ],
          'failure' => 'fail',
         ],
    __DIR__ . '/'
  );

  include(__DIR__.'/autoload-fail-hoist.inc');

  $x = new X();
  $x->method();
  echo 'NOTE: repo-mode doesn\'t invoke the autoloader at all (I1 is in repo)', "\n";
  echo 'Done', "\n";
}
