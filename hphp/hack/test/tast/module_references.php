//// modules_ab.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>

new module a.b {
}

//// modules_ac.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module a.c {
}

//// modules_zc.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module z.c {
}

//// modules_z.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
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
