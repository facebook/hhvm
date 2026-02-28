<?hh
class test {
	function __toString() :mixed{
		return "test object";
	}
}

<<__EntryPoint>>
function entrypoint_bug38261(): void {
  $cert = false;
  $t = new test;

  var_dump(openssl_x509_parse("foo"));
  var_dump(openssl_x509_parse($t));
  var_dump(openssl_x509_parse(vec[]));
  try { var_dump(openssl_x509_parse()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump(openssl_x509_parse($cert));
  var_dump(openssl_x509_parse(new stdClass));
}
