<?php
/**
 * Parsing callback for yaml tag.
 * @param mixed $value Data from yaml file
 * @param string $tag Tag that triggered callback
 * @param int $flags Scalar entity style (see YAML_*_SCALAR_STYLE)
 * @return mixed Value that YAML parser should emit for the given value
 */
function tag_callback ($value, $tag, $flags) {
  var_dump(func_get_args());
  return $tag;
}

$yaml = <<<YAML
implicit_map:
  a: b
explicit_map: !!map
  c: d
implicit_seq: [e, f]
explicit_seq: !!seq [g, h]
YAML;

yaml_parse($yaml, 0, $ndocs, array(
    YAML_MAP_TAG => 'tag_callback',
    YAML_SEQ_TAG => 'tag_callback',
  ));
?>
