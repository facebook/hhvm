<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function convert($d) :mixed{
  var_dump(darray($d));
}

<<__EntryPoint>> function main(): void {
  convert(dict['1' => null]);
  convert(dict['100' => false]);
  convert(dict['123' => true, 123 => false]);
  convert(dict[123 => false, '123' => true]);
  convert(dict[123 => 'a', '456' => 'b', '123' => 'c', 456 => 'd']);
}
