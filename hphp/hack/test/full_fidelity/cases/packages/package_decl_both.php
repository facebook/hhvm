<?hh

<<file:__EnableUnstableFeatures('packages')>>

package x {
  include {
    a,
    b
  }
  use {
    a,
    b.*,
    global
  }
}
