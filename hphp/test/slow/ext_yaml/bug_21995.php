<?php
function yaml_cbk ($a) {
  var_dump($a);
  return $a;
}

$yaml_code = <<<YAML
boo: doo
a: [1,2,3,4]
d: []
YAML;

$yaml = yaml_parse($yaml_code, 0, $ndocs, array(
    YAML_STR_TAG => "yaml_cbk",
    ));
?>
