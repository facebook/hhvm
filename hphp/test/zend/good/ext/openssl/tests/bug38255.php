<?hh

class test {
	function __toString() :mixed{
		return "test object";
	}
}

<<__EntryPoint>>
function entrypoint_bug38255(): void {
  $pub_key_id = false;
  $signature = '';
  $ok = openssl_verify("foo", $signature, $pub_key_id, OPENSSL_ALGO_MD5);
  $t = new test;


  var_dump(openssl_verify("foo", $signature, $pub_key_id, OPENSSL_ALGO_MD5));
  try { var_dump(openssl_verify()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "Done\n";
}
