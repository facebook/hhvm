<?hh

function_call(
  function() {},
  (function() {})(),
  function() { $short_function = 'is short'; },
  function() {
    $longer_function = 'cannot as easily be fit into one line';
    $on_account_of = 'its multiple long statements';
  },
  $foo,
);
