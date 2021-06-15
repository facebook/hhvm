<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class :my:xhp-class {}

<<__EntryPoint>>
function test_xhp() {
  $facts = HH\facts_parse("/", varray[__FILE__], true, false);
  var_dump(vec($facts[__FILE__]));
}
