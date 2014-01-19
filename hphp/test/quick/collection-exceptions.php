<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function go($c, $k) {
  try {
    $unused = $c[$k];
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    $c[$k] = 0;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  var_dump($c);
}

go(Vector {'zero', 'one'}, 2);
go(Vector {'zero', 'one'}, -2);
go(Pair {'zero', 'one'}, 2);
