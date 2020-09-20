<?hh

class test {
	function __toString() {
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
  try { var_dump(openssl_verify("foo", $t, $pub_key_id, OPENSSL_ALGO_MD5)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_verify("foo", new stdClass, $pub_key_id, OPENSSL_ALGO_MD5)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_verify("foo", new stdClass, varray[], OPENSSL_ALGO_MD5)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_verify("foo", varray[], varray[], OPENSSL_ALGO_MD5)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_verify()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_verify(new stdClass, new stdClass, varray[], 10000)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "Done\n";
}
