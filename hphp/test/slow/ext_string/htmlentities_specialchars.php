<?hh

<<__EntryPoint>>
function main_htmlentities_specialchars() :mixed{
$s = chr(0xAE); // this is an ISO-8859-1 circle R
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-1'));
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE)); // UTF-8
$s = chr(0xFF); // this is an ISO-8859-1 umlaut
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-1'));
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE)); // UTF-8
$s = chr(0xFF); // this is an ISO-8859-1 umlaut
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-1'));
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE)); // UTF-8
// this is an ISO-8859-1 para sign + 1/4 + "ABC"
$s = chr(0xB6) . chr(0xBC) . "AAA";
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-1'));
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE)); // UTF-8
// this is an ISO-8859-1 currency sign, but ISO-8859-15 euro sign
$s = chr(0xA4);
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-1'));
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE)); // UTF-8
// This works in PHP 5.x currently, but fatals in HHVM right now
var_dump(htmlentities( $s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-15'));
}
