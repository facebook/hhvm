<?php

$element = simplexml_load_string(<<<EOF
<root>
  <hi hello="world">
    <awesome />
  </hi>
</root>
EOF
);

var_dump($element->hi->attributes());
var_dump($element->hi->attributes()->hello);
