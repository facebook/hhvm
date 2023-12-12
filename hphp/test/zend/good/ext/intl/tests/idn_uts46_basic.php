<?hh
<<__EntryPoint>>
function main_entry(): void {
  $utf8dn = "www.fu\xc3\x9fball.com";
  $asciiNonTrans = "www.xn--fuball-cta.com";

  echo "all ok, no details:", "\n";
  var_dump(idn_to_ascii($utf8dn,
  	IDNA_NONTRANSITIONAL_TO_ASCII, INTL_IDNA_VARIANT_UTS46));

  echo "all ok, no details, transitional:", "\n";
  var_dump(idn_to_ascii($utf8dn, 0, INTL_IDNA_VARIANT_UTS46));

  echo "all ok, with details:", "\n";
  var_dump(idn_to_ascii($utf8dn, IDNA_NONTRANSITIONAL_TO_ASCII,
  	INTL_IDNA_VARIANT_UTS46));

  echo "reverse, ok, with details:", "\n";
  var_dump(idn_to_utf8($asciiNonTrans, 0, INTL_IDNA_VARIANT_UTS46));
}
