<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() {
  var_dump(json_encode(varray[]));
  var_dump(json_encode(varray[1, 2, 3]));

  // empty - logs provenance
  var_dump(json_encode(darray[]));
  // list-like - logs provenance
  var_dump(json_encode(darray[0 => 'a']));
  // map-like
  var_dump(json_encode(darray[5 => 'a']));
}
