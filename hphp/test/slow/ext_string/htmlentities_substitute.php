<?hh


<<__EntryPoint>>
function main_htmlentities_substitute() :mixed{
$foo = varray[
  varray['41efbfbd2667743b42', "\x41\xC2\x3E\x42", null],
  varray['41efbfbd2667743b42', "\x41\xC2\x3E\x42", ENT_NOQUOTES],
  varray['41efbfbd2667743b42', "\x41\xC2\x3E\x42", ENT_QUOTES],
  varray['efbfbd2671756f743b', "\xE3\x80\x22", null],
  varray['efbfbd22', "\xE3\x80\x22", ENT_NOQUOTES],
  varray['efbfbd2671756f743b', "\xE3\x80\x22", ENT_QUOTES],
  varray['41efbfbdefbfbd42efbfbd43e298baefbfbd', "\x41\x98\xBA\x42\xE2\x98\x43\xE2\x98\xBA\xE2\x98", null],
  varray['41efbfbdefbfbd42efbfbd43e298baefbfbd', "\x41\x98\xBA\x42\xE2\x98\x43\xE2\x98\xBA\xE2\x98", ENT_NOQUOTES],
  varray['41efbfbdefbfbd42efbfbd43e298baefbfbd', "\x41\x98\xBA\x42\xE2\x98\x43\xE2\x98\xBA\xE2\x98", ENT_QUOTES],
];

foreach ($foo as $arr) {
  list($exp, $str, $flg) = $arr;
  if ($flg === null) {
    $flg = (int)($flg);
    $flg |= ENT_QUOTES;
  }
  $out = bin2hex(htmlentities($str, $flg | ENT_SUBSTITUTE, 'UTF-8', false));
  if ($out !== $exp) {
    $fstring  = ($flg & ENT_NOQUOTES ? "ENT_NOQUOTES" : "ENT_QUOTES");
    $fstring .= "| ENT_SUBSTITUTE";
    echo bin2hex($str), " ==> ", $out, " !== ", $exp, " (", $fstring . ")\n";
  }
}

echo "Done.\n";
}
