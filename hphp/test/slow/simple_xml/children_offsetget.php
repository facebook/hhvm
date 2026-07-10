<?hh


<<__EntryPoint>>
function main_children_offsetget() :mixed{
$element = simplexml_load_string(<<<EOF
<root>
  <hello>world</hello>
</root>
EOF
);

var_dump($element->children()->offsetGet(0)->__toString());
}
