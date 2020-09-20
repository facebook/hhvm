<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function nonliteral($k) {
  list($a, $b) = $k;
  var_dump($a);
  var_dump($b);
}

function literal() {
  list($a, $b) = keyset[0, 1];
  var_dump($a);
  var_dump($b);
}

function literal_throw() {
  list($a, $b, $c) = keyset[100, 200];
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

function main() {
  nonliteral(keyset[0, 1, 2]);
  literal();

  try {
    nonliteral(keyset[100]);
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
