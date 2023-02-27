<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('packages')>>

package pkg1 {
  use {
    a,
    b.*
  }
}

package pkg2 {
  include {
    pkg1
  }
  use {
    b.b1
  }
}

package pkg3 {
  include {
    pkg2
  }
  use {
    c
  }
}
