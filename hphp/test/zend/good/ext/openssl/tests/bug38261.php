<?hh
$cert = false;
class test {
	function __toString() {
		return "test object";
	}
}
$t = new test;

var_dump(openssl_x509_parse("foo"));
var_dump(openssl_x509_parse($t));
var_dump(openssl_x509_parse(varray[]));
try { var_dump(openssl_x509_parse()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(openssl_x509_parse($cert));
var_dump(openssl_x509_parse(new stdClass));

