<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('packages')>>

package a {
  use {
    x,
    y.*,
    global
  }
}

package b {
  include {
    a
  }
  use {
    y.z
  }
}
