<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :my:xhp-class {}

<<__EntryPoint>>
function test_xhp() :mixed{
  var_dump(HH\Facts\path_to_types(__FILE__));
}
