<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function convert($k) {
  var_dump((array)$k);
}

function main() {
  convert(keyset['1']);
  convert(keyset['100']);
  convert(keyset['123', 123]);
  convert(keyset[123, '123']);
  convert(keyset[123, '456', '123', 456]);
}
main();
