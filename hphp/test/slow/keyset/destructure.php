<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function nonliteral($k) :mixed{
  list($a, $b) = $k;
  var_dump($a);
  var_dump($b);
}

function literal() :mixed{
  list($a, $b) = keyset[0, 1];
  var_dump($a);
  var_dump($b);
}

function literal_throw() :mixed{
  list($a, $b, $c) = keyset[100, 200];
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

function main() :mixed{
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
function main_destructure() :mixed{
main();
}
