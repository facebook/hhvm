<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

set_error_handler(($_, $errmsg) ==> {
  throw new Exception($errmsg);
});

function main() {
  var_dump(varray[vec[]] === varray[varray[]]);
}

try {
  main();
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
}
