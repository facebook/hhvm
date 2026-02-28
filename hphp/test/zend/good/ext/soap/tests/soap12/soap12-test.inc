<?hh
class SimpleStruct {
	public function __construct(
		public string $varString,
		public int $varInt,
		public float $varFloat){}
}

class Soap12test {
	public $header;

	function echoOk($x) {
  	return $x;
	}

	function echoString($inputString) {
		return $inputString;
	}

	function echoStringArray($inputStringArray) {
		return $inputStringArray;
	}

	function echoInteger($inputInteger) {
		return $inputInteger;
	}

	function echoIntegerArray($inputIntegerArray) {
		return $inputIntegerArray;
	}

	function echoFloat($inputFloat) {
		return $inputFloat;
	}

	function echoFloatArray($inputFloatArray) {
		return $inputFloatArray;
  }

	function echoStruct($x) {
  	return $x;
	}

	function echoStructArray($x) {
  	return $x;
	}

	function echoVoid() {
		return NULL;
	}

	function echoBase64($b_encoded) {
		return $b_encoded;
  }

	function echoDate($timeInstant) {
		return $timeInstant;
	}

	function echoHexBinary($hb) {
		return $hb;
	}

	function echoDecimal($dec) {
		return $dec;
	}

	function echoBoolean($boolean) {
		return $boolean;
	}

	function echoStructAsSimpleTypes ($struct) {
		 return dict['outputString'  => $struct->varString,
		              'outputInteger' => $struct->varInt,
		              'outputFloat'   => $struct->varFloat];
	}

	function echoSimpleTypesAsStruct($string, $int, $float) {
		return new SimpleStruct($string, $int, $float);
	}

	function echoNestedStruct($struct) {
		return $struct;
	}

	function echo2DStringArray($ary) {
		return $ary;
	}

	function echoNestedArray($ary) {
		return $ary;
	}

	function countItems($input) {
		return count($input);
	}

	function isNil($input) {
		return is_null($input);
	}

	function returnVoid() {
	}

	function emptyBody() {
	}

	function requiredHeader($x) {
		$this->header = $x;
	}

	function echoHeader() {
	  return $this->header;
	}

	function echoResolvedRef($ref) {
	  return $ref->RelativeReference->base.$ref->RelativeReference->href;
	}

	function validateCountryCode($code) {
		if (strlen($code) != 2) {
		  return new SoapFault("Client", "Not a valid country code", NULL, NULL, NULL, new SoapHeader("http://example.org/ts-tests", "validateCountryCodeFault", "Country code must be 2 letters."));
		} else {
			return "OK";
		}
	}

}

function test() {
	ini_set("soap.wsdl_cache_enabled",0);
	$server = new SoapServer(dirname(__FILE__)."/soap12-test.wsdl", dict['soap_version'=>SOAP_1_2,'actor'=>"http://example.org/ts-tests/C"]);
	$server->setClass("Soap12test");

	$server->handle(\HH\global_get('HTTP_RAW_POST_DATA'));
	echo "ok\n";
}
