<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EnumClass>>
class EC { const X = new stdClass; }
class C { const X = EC::X; }

<<__EntryPoint>>
function enum_class03() :mixed{
  var_dump(C::X); // Exception here.
}
