<?hh

<<file:__EnableUnstableFeatures('module_references')>>

new module a.b {
}

new module a.c {
}

new module z.c {
}

new module a {
  exports {
  }
  imports {
  }
}

new module b {
  exports {
  }
}

new module c {
  imports {
  }
}

new module z {
  imports {
    a.*,
    self.c,
    global,
  }
  exports {
    z.*
  }
}
