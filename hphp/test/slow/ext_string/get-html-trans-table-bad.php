<?php


<<__EntryPoint>>
function main_get_html_trans_table_bad() {
$encodings = array(
  null,
  '',
  5
);

foreach ($encodings as $encoding) {
  $a = get_html_translation_table(HTML_SPECIALCHARS, ENT_COMPAT, $encoding);
  #  var_dump(count($a));
  ksort($a);
  var_dump($a);
}
}
