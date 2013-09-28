<?php
class Emit008Example {
  public $data;    // data may be in any pecl/yaml suitable type

  /**
   * Yaml emit callback function, referred on yaml_emit call by class name.
   *
   * Expected to return an array with 2 values:
   *   - 'tag': custom tag for this serialization
   *   - 'data': value to convert to yaml (array, string, bool, number)
   *
   * @return array
   */
  public static function yamlEmit (Emit008Example $obj) {
    return array(
      'tag' => '!emit008',
      'data' => $obj->data,
    );
  }
}

$emit_callbacks = array(
  'Emit008Example' => array('Emit008Example', 'yamlEmit')
);

$t = new Emit008Example();
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
function parse_008 ($value, $tag, $flags) {
  $ret = new Emit008Example();
  $ret->data = $value;
  return $ret;
}
$parse_callbacks = array(
  '!emit008' => 'parse_008',
);
$array = yaml_parse($yaml, 0, $cnt, $parse_callbacks);
var_dump($array['callback'] == $t);

/* roundtrip with raw object */
var_dump($t == yaml_parse(
  yaml_emit($t, YAML_ANY_ENCODING, YAML_ANY_BREAK, $emit_callbacks),
  0, $cnt, $parse_callbacks));
?>
