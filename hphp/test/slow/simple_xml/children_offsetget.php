<?php

$element = simplexml_load_string(<<<EOF
<root>
  <hello>world</hello>
</root>
EOF
);

var_dump((string)$element->children()[0]);
