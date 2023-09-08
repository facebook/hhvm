<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module a.b {
}

new module a.c {
}

new module z.c {
}

new module z {
  imports {
    a.*,
    self.c,
    global,
  }
  exports {
    a.*
  }
}
