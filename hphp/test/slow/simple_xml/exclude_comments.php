<?php

$element = simplexml_load_string(<<<EOF
<root>
  <!-- I am a comment -->
  <elem1>
    <elem2 />
    <!-- I am also a comment -->
  </elem1>
</root>
EOF
);
var_dump($element->count());
var_dump($element->elem1->count());
