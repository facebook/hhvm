<?php
$utf8dn = "www.fußball.com";
$asciiNonTrans = "www.xn--fuball-cta.com";

echo "all ok, no details:", "\n";
var_dump(idn_to_ascii($utf8dn, 
	IDNA_NONTRANSITIONAL_TO_ASCII, INTL_IDNA_VARIANT_UTS46));
	
echo "all ok, no details, transitional:", "\n";
var_dump(idn_to_ascii($utf8dn, 0, INTL_IDNA_VARIANT_UTS46));

echo "all ok, with details:", "\n";
var_dump(idn_to_ascii($utf8dn, IDNA_NONTRANSITIONAL_TO_ASCII,
	INTL_IDNA_VARIANT_UTS46, $info));
var_dump($info);

echo "reverse, ok, with details:", "\n";
var_dump(idn_to_utf8($asciiNonTrans, 0, INTL_IDNA_VARIANT_UTS46, $info));
var_dump($info);