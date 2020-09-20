<?hh
<<__EntryPoint>> function main(): void {
  $m = null;
  var_dump(preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\])?\s*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_SET_ORDER,
  ));
  var_dump($m);

  var_dump(preg_match_all_with_matches(
    '/(?:\([^)]+\))?(&?)([\w>.()-]+(?:\[\w+\])?)\s*,?((?:\)*\s*=)?)/S',
    '&a, b, &c',
    inout $m,
    PREG_SET_ORDER,
  ));
  var_dump($m);

  var_dump(preg_match_all_with_matches(
    '/zend_parse_parameters(?:_ex\s*\([^,]+,[^,]+|\s*\([^,]+),\s*"([^"]*)"\s*,\s*([^{;]*)/S',
    'zend_parse_parameters( 0, "addd|s/", a, b, &c);',
    inout $m,
    PREG_SET_ORDER | PREG_OFFSET_CAPTURE,
  ));
  var_dump($m);

var_dump(preg_split('/PHP_(?:NAMED_)?(?:FUNCTION|METHOD)\s*\((\w+(?:,\s*\w+)?)\)/S', "PHP_FUNCTION(s, preg_match)\n{\nlalala", -1, PREG_SPLIT_DELIM_CAPTURE | PREG_SPLIT_OFFSET_CAPTURE));
}
