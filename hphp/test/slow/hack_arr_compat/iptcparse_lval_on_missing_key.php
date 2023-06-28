<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_iptcparse_lval_on_missing_key() :mixed{
  $parsed = iptcparse("\034\002x\000\003foo\034\002y\000\003bar");
  var_dump($parsed !== false);
}
