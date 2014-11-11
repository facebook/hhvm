<?php
$xml = new SimpleXMLElement(<<<EOF
<foo>
    <bar />
    <baz hello="" />
    <bak hello="world" />
    <bat><bar /></bat>
    <baq><bar hello="world" /></baq>
</foo>
EOF
);

var_dump(empty($xml->bar));
var_dump(empty($xml->baz));
var_dump(empty($xml->bak));
var_dump(empty($xml->bat));
var_dump(empty($xml->baq));
