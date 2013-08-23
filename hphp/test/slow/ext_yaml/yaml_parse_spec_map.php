<?php
  var_dump(yaml_parse('
# Unordered set of key: value pairs.
Block style: !!map
  Clark : Evans
  Brian : Ingerson
  Oren  : Ben-Kiki
Flow style: !!map { Clark: Evans, Brian: Ingerson, Oren: Ben-Kiki }
'));
?>
