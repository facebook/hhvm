<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function nonliteral($d) {
  list($a, $b) = $d;
  var_dump($a);
  var_dump($b);
}

function literal() {
  list($a, $b) = dict[0 => 'abc', 1 => 'def'];
  var_dump($a);
  var_dump($b);
}

function literal_throw() {
  list($a, $b, $c) = dict[100 => 'abc', 200 => 'def'];
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

function main() {
  nonliteral(dict[0 => 0, 1 => 1, 2 => 2]);
  literal();

  try {
    nonliteral(dict[100 => 'abc']);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }

  try {
    literal_throw();
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main_destructure() {
main();
}
