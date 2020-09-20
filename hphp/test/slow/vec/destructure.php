<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function nonliteral($v) {
  list($a, $b) = $v;
  var_dump($a);
  var_dump($b);
}

function literal() {
  list($a, $b) = vec['abc', 'def'];
  var_dump($a);
  var_dump($b);
}

function literal_throw() {
  list($a, $b, $c) = vec['abc', 'def'];
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

function main() {
  nonliteral(vec[1, 2, 3]);
  literal();

  try {
    nonliteral(vec[1]);
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
