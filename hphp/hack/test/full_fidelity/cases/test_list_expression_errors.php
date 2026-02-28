<?hh

// legal because 'list' can be the value clause of a foreach
foreach($array as list($odd, $even)){
}

// legal because 'list' can be the operand to a list destructuring
list($a, list($b, $c)) = $vector;

// legal because nested destructuring, and use as value clause, are both legal
foreach($array as list($odd, list($even))){
}

// legal because used as left side of simple assignment
list($a) = $vector;
list(,,$a,,) = $vector;
// legal because simple assignment is right-associative
$x = list($whatever) = $whatever;

foo(list($a, $b)); // error 2040
list(2) + 3; // error 2040
list(2) % 3; // error 2040

function foo(): (int, int) {
  return list(1, 2); // error, list can only be used as an lvar
}
