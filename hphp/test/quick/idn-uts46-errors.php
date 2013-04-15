<?php
  ini_set("intl.error_level", E_WARNING);
echo "=> PHP level errors", "\n";

echo "bad args:", "\n";
var_dump(idn_to_ascii("", 0, array()));
var_dump(idn_to_ascii("", 0, INTL_IDNA_VARIANT_UTS46, $foo, null));

echo "bad variant:", "\n";
var_dump(idn_to_ascii("", 0, INTL_IDNA_VARIANT_UTS46 + 10));

echo "empty domain:", "\n";
var_dump(idn_to_ascii("", 0, INTL_IDNA_VARIANT_UTS46));

echo "fourth arg for 2003 variant (only notice raised):", "\n";
var_dump(idn_to_ascii("foo.com", 0, INTL_IDNA_VARIANT_2003, $foo));

echo "with error, but no details arg:", "\n";
var_dump(idn_to_ascii("www.fußball.com-", 0, INTL_IDNA_VARIANT_UTS46));

echo "with error, with details arg:", "\n";
var_dump(idn_to_ascii("www.fußball.com-", IDNA_NONTRANSITIONAL_TO_ASCII,
                      INTL_IDNA_VARIANT_UTS46, $foo));
var_dump($foo);

echo "with error, with details arg, contextj:", "\n";
var_dump(idn_to_ascii(
           html_entity_decode("www.a&#x200D;b.com", 0, "UTF-8"),
           IDNA_NONTRANSITIONAL_TO_ASCII | IDNA_CHECK_CONTEXTJ,
           INTL_IDNA_VARIANT_UTS46, $foo));
var_dump($foo);
var_dump($foo["errors"]==IDNA_ERROR_CONTEXTJ);
