<?php
$emit_callbacks = array(
  'Emit009Example' => function ($o) {
    return array(
      'tag' => '!emit009',
      'data' => $o->data,
    );
  },
);

class Emit009Example {
  public $data;    // data may be in any pecl/yaml suitable type
}

$t = new Emit009Example();
$t->data = array ('a','b','c');
$yaml = yaml_emit(
  array(
    'callback' => $t,
  ),
  YAML_ANY_ENCODING,
  YAML_ANY_BREAK,
  $emit_callbacks
);
var_dump($yaml);

/* make sure you can undo the custome serialization */
function parse_009 ($value, $tag, $flags) {
  $ret = new Emit009Example();
  $ret->data = $value;
  return $ret;
}
$parse_callbacks = array(
  '!emit009' => 'parse_009',
);
$array = yaml_parse($yaml, 0, $cnt, $parse_callbacks);
var_dump($array['callback'] == $t);

/* roundtrip with raw object */
var_dump($t == yaml_parse(
  yaml_emit($t, YAML_ANY_ENCODING, YAML_ANY_BREAK, $emit_callbacks),
  0, $cnt, $parse_callbacks));
?>
