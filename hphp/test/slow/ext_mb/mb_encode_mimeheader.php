<?hh


<<__EntryPoint>>
function main_mb_encode_mimeheader() :mixed{
mb_internal_encoding("ISO-8859-1");
$expected_result = "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=";
var_dump($expected_result === mb_encode_mimeheader(
  "Subject: Pr\xDC"."fung Pr\xDC"."fung"
));
var_dump($expected_result === mb_encode_mimeheader(
  "Subject: Pr\xDC"."fung Pr\xDC"."fung",
  "UTF-8"
));
var_dump($expected_result === mb_encode_mimeheader(
  "Subject: Pr\xDC"."fung Pr\xDC"."fung",
  "UTF-8",
  "B"
));
var_dump($expected_result === mb_encode_mimeheader(
  "Subject: Pr\xDC"."fung Pr\xDC"."fung",
  "UTF-8",
  "B",
  "\r\n"
));
var_dump($expected_result === mb_encode_mimeheader(
  "Subject: Pr\xDC"."fung Pr\xDC"."fung",
  "UTF-8",
  "B",
  "\n",
  0
));
mb_internal_encoding("UTF-8");
}
