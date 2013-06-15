<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x, true); }

//////////////////////////////////////////////////////////////////////

function test_iconv_mime_encode() {
  $preferences = array("input-charset" => "ISO-8859-1",
                                  "output-charset" => "UTF-8",
                                  "line-length" => 76,
                                  "line-break-chars" => "\n");
  $preferences['scheme'] = "Q";
  VS(iconv_mime_encode("Subject", "Pr\xDC"."fung Pr\xDC"."fung", $preferences),
     "Subject: =?UTF-8?Q?Pr=C3=9Cfung=20Pr=C3=9Cfung?=");

  $preferences['scheme'] = "B";
  VS(iconv_mime_encode("Subject", "Pr\xDC"."fung Pr\xDC"."fung", $preferences),
     "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=");
}

function test_iconv_mime_decode() {
  VS(iconv_mime_decode("=?windows-1256?Q?My_Persona?= =?windows-1256?Q?l_Profile_?= =?windows-1256?Q?was_Disabl?= =?windows-1256?Q?ed=FE?="), false);

  VS(iconv_mime_decode("Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=",
                         0, "ISO-8859-1"),
     "Subject: Pr\xDC"."fung Pr\xDC"."fung");

  VS(iconv_mime_decode(
       "Subject: =?utf-8?Q?JS_typeahead_should_normalize_Polish_=C5=82_=3C-=3E_".
       "l_?=\n =?utf-8?Q?=28S=C5=82awek_Biel=29?=",
       0,
       "UTF-8"),
     "Subject: JS typeahead should normalize Polish \xC5\x82 <-> l (S\xC5\x82awek ".
     "Biel)");
}

function test_iconv_mime_decode_headers() {
  VS(iconv_mime_decode_headers
     ("Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=\n".
      "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=\n",
      0, "ISO-8859-1"),
     array("Subject" => array("Pr\xDC"."fung Pr\xDC"."fung",
                                           "Pr\xDC"."fung Pr\xDC"."fung")));
}

function test_iconv_get_encoding() {
  VS(iconv_get_encoding(),
     array("input_encoding" => "ISO-8859-1",
                 "output_encoding" => "ISO-8859-1",
                 "internal_encoding" => "ISO-8859-1"));
}

function test_iconv_set_encoding() {
  VS(iconv_set_encoding("output_encoding", "UTF-8"), true);

  VS(iconv_get_encoding(),
     array("input_encoding" => "ISO-8859-1",
                 "output_encoding" => "UTF-8",
                 "internal_encoding" => "ISO-8859-1"));
}

function test_iconv() {
  VS(iconv("UTF-8", "BIG5", "\xE2\x82\xAC"), "\xa3\xe1");
  VS(iconv("ISO-8859-1", "UTF-8", "Pr\xDC"."fung"), "Pr\xC3\x9C"."fung");
}

function test_iconv_strlen() {
  VS(iconv_strlen("Pr\xDC"."fung", "ISO-8859-1"), 7);
  VS(iconv_strlen("Pr\xC3\x9C"."fung", "UTF-8"), 7);
}

function test_iconv_strpos() {
  VS(iconv_strpos("Pr\xC3\x9C\xC3\x9D"."fung", "\xC3\x9D", 0, "UTF-8"), 3);
}

function test_iconv_strrpos() {
  VS(iconv_strrpos("Pr\xC3\x9C"."abc\xC3\x9C"."fung", "\xC3\x9C", "UTF-8"), 6);
}

function test_iconv_substr() {
  VS(iconv_substr("Pr\xC3\x9C\xC3\x9D"."fung", 2, 2, "UTF-8"),
     "\xC3\x9C\xC3\x9D");
}

test_iconv_mime_encode();
test_iconv_mime_decode();
test_iconv_mime_decode_headers();
test_iconv_get_encoding();
test_iconv_set_encoding();
test_iconv();
test_iconv_strlen();
test_iconv_strpos();
test_iconv_strrpos();
test_iconv_substr();
