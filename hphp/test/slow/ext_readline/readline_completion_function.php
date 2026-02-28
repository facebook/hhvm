<?hh

function foo() :mixed{ }


<<__EntryPoint>>
function main_readline_completion_function() :mixed{
$data = vec[
  foo<>,
  strtolower<>,
  1,
  1.1231,
  function ($str, $start, $end) { return vec[]; },
];

foreach ($data as $callback) {
  readline_completion_function($callback);
}
}
