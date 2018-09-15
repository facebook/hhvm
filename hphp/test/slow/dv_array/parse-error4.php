<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a) {
  $v = varray[&$a];
  var_dump($v);
}

<<__EntryPoint>>
function main_parse_error4() {
main(100);
}
