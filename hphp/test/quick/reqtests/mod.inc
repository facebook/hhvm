<?php
echo "in mod.php\n";
if (isset($this) && isset($this->var)) {
  echo "this->var was set; changing it.\n";
  $this->var .= ' more';
}

// test inheriting the varenv
$testvar = 'hello';
