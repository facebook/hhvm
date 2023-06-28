<?hh

function foo() :mixed{ }


<<__EntryPoint>>
function main_readline_completion_function() :mixed{
$data = varray[
  foo<>,
  strtolower<>,
  1,
  1.1231,
  function ($str, $start, $end) { return varray[]; },
];

foreach ($data as $callback) {
  readline_completion_function($callback);
}
}
