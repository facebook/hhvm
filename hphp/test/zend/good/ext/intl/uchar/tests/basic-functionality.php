<?hh

function unicode_info($cp) :mixed{
  $methods = dict[
    'charName' => IntlChar::charName<>,

    'isUAlphabetic' => IntlChar::isUAlphabetic<>,
    'isalpha' => IntlChar::isalpha<>,
    'isalnum' => IntlChar::isalnum<>,
    'isbase' => IntlChar::isbase<>,
    'isdigit' => IntlChar::isdigit<>,
    'isxdigit' => IntlChar::isxdigit<>,

    'isULowercase' => IntlChar::isULowercase<>,
    'isUUppercase' => IntlChar::isUUppercase<>,
    'islower' => IntlChar::islower<>,
    'isupper' => IntlChar::isupper<>,
    'istitle' => IntlChar::istitle<>,

    'isUWhiteSpace' => IntlChar::isUWhiteSpace<>,
    'isWhitespace' => IntlChar::isWhitespace<>,
    'isblank' => IntlChar::isblank<>,
    'iscntrl' => IntlChar::iscntrl<>,
    'isdefined' => IntlChar::isdefined<>,
    'isgraph' => IntlChar::isgraph<>,
    'isprint' => IntlChar::isprint<>,
    'ispunct' => IntlChar::ispunct<>,
    'isspace' => IntlChar::isspace<>,

    'isIDIgnorable' => IntlChar::isIDIgnorable<>,
    'isIDPart' => IntlChar::isIDPart<>,
    'isIDStart' => IntlChar::isIDStart<>,
    'isISOControl' => IntlChar::isISOControl<>,
    'isJavaIDPart' => IntlChar::isJavaIDPart<>,
    'isJavaIDStart' => IntlChar::isJavaIDStart<>,
    'isJavaSpaceChar' => IntlChar::isJavaSpaceChar<>,
    'isMirrored' => IntlChar::isMirrored<>,

    'getBlockCode' => IntlChar::getBlockCode<>,
  ];

  $props = vec[
    IntlChar::PROPERTY_ALPHABETIC,
    IntlChar::PROPERTY_BLOCK,
  ];

  $ncp = IntlChar::ord($cp);
  printf("Codepoint U+%04x\n", $ncp);

  foreach($methods as $method => $func) {
    echo "  $method(): ";
    var_dump($func($cp));
  }

  foreach($props as $prop) {
    printf("  hasBinaryProperty(%s): %s\n",
      IntlChar::getPropertyName($prop),
      IntlChar::hasBinaryProperty($cp, $prop) ? "true" : "false"
    );
  }

  $age = IntlChar::charAge($cp);
  $age = ($age is null) ? "null" : implode('.', $age);
  echo "  charAge(): ", $age, "\n";
  echo "\n";
}

<<__EntryPoint>>
function main(): void {
  printf(
    "Codepoint range: %04x-%04x\n\n",
    IntlChar::CODEPOINT_MIN,
    IntlChar::CODEPOINT_MAX,
  );
  $codepoints = vec['E', 0xDF, 0x2603, IntlChar::chr(0x3000)];
  foreach ($codepoints as $cp) {
    unicode_info($cp);
  }
  echo "Sample range of codepoints: U+2600-U+260F\n";
  IntlChar::enumCharNames(
    0x2600,
    0x2610,
    function($cp, $nc, $name) {
      printf("U+%04x %s\n", $cp, $name);
    },
  );
  echo "\n";
  echo "RECYCLING SYMBOL FOR TYPE-1 PLASTICS => ";
  var_dump(IntlChar::charFromName("RECYCLING SYMBOL FOR TYPE-1 PLASTICS"));
}
