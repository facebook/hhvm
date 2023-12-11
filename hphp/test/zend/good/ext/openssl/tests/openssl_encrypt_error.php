<?hh <<__EntryPoint>> function main(): void {
$data = "openssl_encrypt() tests";
$method = "AES-128-CBC";
$password = "openssl";
$wrong = "wrong";
$object = new stdClass;
$arr = vec[1];

var_dump(openssl_encrypt($data, $wrong, $password));
try { var_dump(openssl_encrypt($object, $method, $password)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_encrypt($data, $object, $password)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_encrypt($data, $method, $object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_encrypt($arr, $method, $object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_encrypt($data, $arr, $object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(openssl_encrypt($data, $method, $arr)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
