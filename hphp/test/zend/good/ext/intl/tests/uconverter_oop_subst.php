<?php
$c = new UConverter('ascii', 'utf-8');

foreach(array('?','','<unknown>') as $subst) {
  if (!$c->setSubstChars($subst)) {
    echo "**Disallowed\n";
    continue;
  }
  var_dump($c->convert("This is an ascii string"));
  var_dump($c->convert("Snowman: (\xE2\x98\x83)"));
}
