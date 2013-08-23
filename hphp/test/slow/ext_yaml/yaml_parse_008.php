<?php
/**
 * Parsing callback for yaml tag.
 * @param mixed $value Data from yaml file
 * @param string $tag Tag that triggered callback
 * @param int $flags Scalar entity style (see YAML_*_SCALAR_STYLE)
 * @return mixed Value that YAML parser should emit for the given value
 */
function tag_callback ($value, $tag, $flags) {
  if (is_array($value)) {
    $new_value = '';
    foreach ($value as $k => $v) {
      $new_value .= "'{$k}' => '{$v}', ";
    }
    $value = trim($new_value, ', ');
  }
  return "<value=[{$value}], tag=[{$tag}], flags=[{$flags}]>";
}

// yaml with some custom tags
$yaml_str = <<<YAML
%TAG ! test-
%TAG !! test2-
---
key_a : !tag_a value_a
key_b : !!tag_b 'value_b'
key_c : !<tag:example.com,2011:test/tag_c> "value_c"
key_d : !tag_d |
  some
  text
key_e : !tag_e >
  some
  text
key_f : !tag_f [ one, two ]
key_g : !tag_g { sky: blue, sea: green }
key_h : !tag_h
  - one
  - two
key_i : !tag_i
  sky: blue
  sea: green
...
YAML;


$yaml = yaml_parse($yaml_str, 0, $ndocs, array(
    "test-tag_a" => "tag_callback",
    "test2-tag_b" => "tag_callback",
    "tag:example.com,2011:test/tag_c" => "tag_callback",
    "test-tag_d" => "tag_callback",
    "test-tag_e" => "tag_callback",
    "test-tag_f" => "tag_callback",
    "test-tag_g" => "tag_callback",
    "test-tag_h" => "tag_callback",
    "test-tag_i" => "tag_callback",
  ));

var_dump($yaml);
var_dump($ndocs);
?>
