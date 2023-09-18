//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>

new module a.b {
}

//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module a.c {
}

//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module z.c {
}

//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module a {
  exports {
  }
  imports {
  }
}

//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module b {
  exports {
  }
}

//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
new module c {
  imports {
  }
}

//// modules.php
<?hh
<<file:__EnableUnstableFeatures('module_references')>>
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
