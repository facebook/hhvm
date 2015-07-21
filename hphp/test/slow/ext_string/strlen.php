<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  var_dump(strlen(null));
  var_dump(strlen(true));
  var_dump(strlen(false));

  var_dump(strlen(123456));
  var_dump(strlen(123456.0));
  var_dump(strlen(123.456));

  var_dump(strlen(array()));
  var_dump(strlen(array("str")));

  var_dump(strlen("null"));
  var_dump(strlen("true"));
  var_dump(strlen("false"));
  var_dump(strlen("123456"));
  var_dump(strlen("123456.0"));
  var_dump(strlen("123.456"));
  var_dump(strlen("array()"));
  var_dump(strlen("array('str')"));
}

main();
