<?hh

function foo() { }


<<__EntryPoint>>
function main_readline_completion_function() {
$data = varray[
  'foo',
  'strtolower',
  1,
  1.1231,
  function ($str, $start, $end) { return varray[]; },
];

foreach ($data as $callback) {
  readline_completion_function($callback);
}
}
