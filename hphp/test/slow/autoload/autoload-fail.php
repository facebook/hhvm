<?hh

function fail($t, $n, $e) {
  var_dump($t, $n);
  if ($e is Exception) {
    var_dump($e->getMessage(), $e->getTrace());
  } else {
    var_dump($e);
  }
  if ($n == 'C') {
    include 'autoload-fail-1.inc';
  } else if ($n == 'D') {
    include 'autoload-fail-2.inc';
  }
}
<<__EntryPoint>>
function entrypoint_autoloadfail(): void {

  \HH\autoload_set_paths(
    darray['class' => darray[
            'c' => 'autoload-fail-c.inc', // syntax error
          ],
          'failure' => 'fail',
         ],
    __DIR__ . '/'
  );

  $x = new C;
}
