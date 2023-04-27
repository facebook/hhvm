<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

enum E : string as string { A = "A"; }

function getLikeE(): ~E {
  return E::A;
}

function foo(): (~E & string) {
  // Should fail
  return getLikeE();
}
