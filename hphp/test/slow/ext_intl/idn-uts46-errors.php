<?hh

// Php doesn't support \u escapes.
function u($x) :mixed{ return json_decode("\"" . $x . "\""); }


<<__EntryPoint>>
function main_idn_uts46_errors() :mixed{
echo "bad variant:", "\n";
var_dump(idn_to_ascii("foo.com", 0, INTL_IDNA_VARIANT_UTS46 + 10));

echo "\n", "leading hyphen, but no details arg:", "\n";
var_dump(idn_to_ascii("-foo.com", 0, INTL_IDNA_VARIANT_UTS46));

echo "\n", "empty domain:", "\n";
var_dump(idn_to_ascii("", 0, INTL_IDNA_VARIANT_UTS46));

echo "\n", "domain too long (max allowed 253, trailing hyphen ignored):", "\n";
$result = idn_to_ascii(
  str_repeat("a.", 126) . "aa",
  0,
  INTL_IDNA_VARIANT_UTS46,
);
var_dump($result);

echo "\n", "buffer overflow:", "\n";
$result = idn_to_ascii(
  str_repeat("a", 2048),
  0,
  INTL_IDNA_VARIANT_UTS46,
);
var_dump($result);
var_dump(intl_get_error_code() == U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR);

echo "\n", "BiDi and CONTEXTJ errors:", "\n";
$result = idn_to_ascii(
  u("\\u0644\\u200C"),
  IDNA_NONTRANSITIONAL_TO_ASCII | IDNA_CHECK_BIDI | IDNA_CHECK_CONTEXTJ,
  INTL_IDNA_VARIANT_UTS46,
);
var_dump($result);

echo "\n", "reverse, invalid Punycode:", "\n";
$result = idn_to_utf8(
  "xn--0",
  0,
  INTL_IDNA_VARIANT_UTS46,
);
var_dump($result);

echo "\n", "reverse, buffer overflow:", "\n";
$result = idn_to_utf8(
  str_repeat("a", 2048),
  0,
  INTL_IDNA_VARIANT_UTS46,
);
var_dump($result);
var_dump(intl_get_error_code() == U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR);
}
