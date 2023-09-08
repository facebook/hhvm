<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module x {
}

new module y {
}

new module z {
  imports {
    x
  }
  exports {
    y
  }
}
