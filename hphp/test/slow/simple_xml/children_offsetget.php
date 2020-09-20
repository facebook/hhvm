<?hh


<<__EntryPoint>>
function main_children_offsetget() {
$element = simplexml_load_string(<<<EOF
<root>
  <hello>world</hello>
</root>
EOF
);

var_dump((string)$element->children()->offsetGet(0));
}
