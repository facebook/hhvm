<?hh


<<__EntryPoint>>
function main_get_html_trans_table_bad() :mixed{
$encodings = vec[
  '',
];

foreach ($encodings as $encoding) {
  try {
    $a = get_html_translation_table(HTML_SPECIALCHARS, ENT_COMPAT, $encoding);
    //  var_dump(count($a));
    ksort(inout $a);
    var_dump($a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
}
