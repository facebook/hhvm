<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class StrlenTest {
  public function __toString() {
    return 'StrlenTest__toString!';
  }
}

function main() {
  var_dump(strlen());

  var_dump(strlen(null));
  var_dump(strlen(true));
  var_dump(strlen(false));

  var_dump(strlen(123456));
  var_dump(strlen(123456.0));
  var_dump(strlen(123.456));

  var_dump(strlen(array()));
  var_dump(strlen(array("str")));

  var_dump(strlen(vec[]));
  var_dump(strlen(dict[]));
  var_dump(strlen(keyset[]));

  var_dump(strlen(new stdClass()));
  var_dump(strlen(new StrlenTest()));

  var_dump(strlen("null"));
  var_dump(strlen("true"));
  var_dump(strlen("false"));
  var_dump(strlen("123456"));
  var_dump(strlen("123456.0"));
  var_dump(strlen("123.456"));
  var_dump(strlen("array()"));
  var_dump(strlen("array('str')"));
  var_dump(strlen("new stdClass()"));
}


<<__EntryPoint>>
function main_strlen() {
main();
}
