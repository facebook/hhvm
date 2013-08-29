<?php

$xml = simplexml_load_string(<<<EOF
<root>
  <block name="test" />
  <test>
    <blob>test</blob>
  </test>
</root>
EOF
);

$block = $xml->xpath("//block[@name='test']");
$block[0]->addAttribute('ignore', true);

var_dump((bool)$block[0]->attributes()->ignore);
var_dump((bool)$xml->children()->block->attributes()->ignore);
