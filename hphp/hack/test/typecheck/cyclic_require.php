<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I {
  require implements J;
}
interface J {
  require implements I;
}
