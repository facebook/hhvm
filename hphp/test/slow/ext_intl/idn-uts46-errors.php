<?php

// Php doesn't support \u escapes.
function u($x) { return json_decode("\"" . $x . "\""); }

echo "bad variant:", "\n";
var_dump(idn_to_ascii("foo.com", 0, INTL_IDNA_VARIANT_UTS46 + 10));

echo "\n", "leading hyphen, but no details arg:", "\n";
var_dump(idn_to_ascii("-foo.com", 0, INTL_IDNA_VARIANT_UTS46));

echo "\n", "empty domain:", "\n";
var_dump(idn_to_ascii("", 0, INTL_IDNA_VARIANT_UTS46, $info));
var_dump($info);
var_dump($info["errors"] == IDNA_ERROR_EMPTY_LABEL);

echo "\n", "domain too long (max allowed 253, trailing hyphen ignored):", "\n";
$result = idn_to_ascii(
  str_repeat("a.", 126) . "aa",
  0,
  INTL_IDNA_VARIANT_UTS46,
  $info
);
var_dump($result);
var_dump($info);
var_dump($info["errors"] == IDNA_ERROR_DOMAIN_NAME_TOO_LONG);

echo "\n", "buffer overflow:", "\n";
$result = idn_to_ascii(
  str_repeat("a", 2048),
  0,
  INTL_IDNA_VARIANT_UTS46,
  $info
);
var_dump($result);
var_dump($info);
var_dump(intl_get_error_code() == U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR);

echo "\n", "BiDi and CONTEXTJ errors:", "\n";
$result = idn_to_ascii(
  u("\\u0644\\u200C"),
  IDNA_NONTRANSITIONAL_TO_ASCII | IDNA_CHECK_BIDI | IDNA_CHECK_CONTEXTJ,
  INTL_IDNA_VARIANT_UTS46,
  $info
);
var_dump($result);
var_dump($info);
var_dump($info["errors"] == (IDNA_ERROR_BIDI | IDNA_ERROR_CONTEXTJ));

echo "\n", "reverse, invalid Punycode:", "\n";
$result = idn_to_utf8(
  "xn--0",
  0,
  INTL_IDNA_VARIANT_UTS46,
  $info
);
var_dump($result);
var_dump($info);
var_dump($info["errors"] == IDNA_ERROR_PUNYCODE);

echo "\n", "reverse, buffer overflow:", "\n";
$result = idn_to_utf8(
  str_repeat("a", 2048),
  0,
  INTL_IDNA_VARIANT_UTS46,
  $info
);
var_dump($result);
var_dump($info);
var_dump(intl_get_error_code() == U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR);
