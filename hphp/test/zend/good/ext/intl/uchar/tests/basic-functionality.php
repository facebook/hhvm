<?php

function unicode_info($cp) {
  $proplist = array(
    IntlChar::PROPERTY_ALPHABETIC,
  );
  $methodList = array(
    'isUAlphabetic',
    'isUUppercase', 'isupper',
    'isULowercase', 'islower',
    'isUWhiteSpace', 'isWhitespace',
    'istitle', 'isdigit', 'isalpha', 'isalnum',
    'isxdigit', 'ispunct', 'ispunct', 'isgraph',
    'isblank', 'isdefined', 'isspace', 'iscntrl',
    'isMirrored', 'isIDStart', 'isIDPart',
    'getBlockCode', 'charName',
  );

  $ncp = IntlChar::ord($cp);
  printf("Codepoint U+%04x\n", $ncp);

  foreach($proplist as $prop) {
    printf("  hasBinaryProperty(%s): %s\n",
      IntlChar::getPropertyName($prop),
      IntlChar::hasBinaryProperty($cp, $prop) ? "true" : "false"
    );
  }
  foreach($methodList as $method) {
    echo "  $method(): ";
    var_dump(IntlChar::$method($cp));
  }
  echo "  charAge(): ", implode('.', IntlChar::charAge($cp)), "\n";
  echo "\n";
}

printf("Codepoint range: %04x-%04x\n", IntlChar::CODEPOINT_MIN, IntlChar::CODEPOINT_MAX);
$codepoints = array('P', 0xDF, 0x2603);
foreach($codepoints as $cp) {
  unicode_info($cp);
}
echo "Sample range of codepoints: U+2600-U+260F\n";
IntlChar::enumCharNames(0x2600, 0x2610, function($cp, $nc, $name) {
  printf("U+%04x %s\n", $cp, $name);
});
echo "RECYCLING SYMBOL FOR TYPE-1 PLASTICS => ";
var_dump(IntlChar::charFromName("RECYCLING SYMBOL FOR TYPE-1 PLASTICS"));
