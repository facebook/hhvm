<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int SOAP_1_1 = 1;
const int SOAP_1_2 = 2;
const int SOAP_ACTOR_NEXT = 1;
const int SOAP_ACTOR_NONE = 2;
const int SOAP_ACTOR_UNLIMATERECEIVER = 3;
const int SOAP_AUTHENTICATION_BASIC = 0;
const int SOAP_AUTHENTICATION_DIGEST = 1;
const int SOAP_COMPRESSION_ACCEPT = 32;
const int SOAP_COMPRESSION_DEFLATE = 16;
const int SOAP_COMPRESSION_GZIP = 0;
const int SOAP_DOCUMENT = 2;
const int SOAP_ENCODED = 1;
const int SOAP_ENC_ARRAY = 300;
const int SOAP_ENC_OBJECT = 301;
const int SOAP_FUNCTIONS_ALL = 999;
const int SOAP_LITERAL = 2;
const int SOAP_PERSISTENCE_REQUEST = 2;
const int SOAP_PERSISTENCE_SESSION = 1;
const int SOAP_RPC = 1;
const int SOAP_SINGLE_ELEMENT_ARRAYS = 1;
const int SOAP_USE_XSI_ARRAY_TYPE = 4;
const int SOAP_WAIT_ONE_WAY_CALLS = 2;

const string XSD_NAMESPACE = 'http://www.w3.org/2001/XMLSchema';
const string XSD_1999_NAMESPACE = 'http://www.w3.org/1999/XMLSchema';

const int XSD_1999_TIMEINSTANT = 401;
const int XSD_ANYTYPE = 145;
const int XSD_ANYURI = 117;
const int XSD_ANYXML = 147;
const int XSD_BASE64BINARY = 116;
const int XSD_BOOLEAN = 102;
const int XSD_BYTE = 137;
const int XSD_DATE = 109;
const int XSD_DATETIME = 107;
const int XSD_DECIMAL = 103;
const int XSD_DOUBLE = 105;
const int XSD_DURATION = 106;
const int XSD_ENTITIES = 130;
const int XSD_ENTITY = 129;
const int XSD_FLOAT = 104;
const int XSD_GDAY = 113;
const int XSD_GMONTH = 114;
const int XSD_GMONTHDAY = 112;
const int XSD_GYEAR = 111;
const int XSD_GYEARMONTH = 110;
const int XSD_HEXBINARY = 115;
const int XSD_ID = 126;
const int XSD_IDREF = 127;
const int XSD_IDREFS = 128;
const int XSD_INT = 135;
const int XSD_INTEGER = 131;
const int XSD_LANGUAGE = 122;
const int XSD_LONG = 134;
const int XSD_NAME = 124;
const int XSD_NCNAME = 125;
const int XSD_NEGATIVEINTEGER = 133;
const int XSD_NMTOKEN = 123;
const int XSD_NMTOKENS = 144;
const int XSD_NONNEGATIVEINTEGER = 138;
const int XSD_NONPOSITIVEINTEGER = 132;
const int XSD_NORMALIZEDSTRING = 120;
const int XSD_NOTATION = 119;
const int XSD_POSITIVEINTEGER = 143;
const int XSD_QNAME = 118;
const int XSD_SHORT = 136;
const int XSD_STRING = 101;
const int XSD_TIME = 108;
const int XSD_TOKEN = 121;
const int XSD_UNSIGNEDBYTE = 142;
const int XSD_UNSIGNEDINT = 140;
const int XSD_UNSIGNEDLONG = 139;
const int XSD_UNSIGNEDSHORT = 141;

const int WSDL_CACHE_NONE = 0;
const int WSDL_CACHE_MEMORY = 2;
const int WSDL_CACHE_DISK = 1;
const int WSDL_CACHE_BOTH = 3;

<<__PHPStdLib>>
function use_soap_error_handler(bool $handler = true);
<<__PHPStdLib>>
function is_soap_fault($fault);
<<__PHPStdLib>>
function _soap_active_version();

class SoapServer {
  public function __construct($wsdl, $options = null);
  public function setclass(string $name, ...$args);
  public function setobject($obj);
  public function addfunction($func);
  public function getfunctions();
  public function handle($request = null);
  public function setpersistence(int $mode);
  public function fault($code, string $fault, $actor = null, $detail = null, $name = null);
  public function addsoapheader($fault);
}

class SoapClient {
  public function __construct($wsdl, $options = null);
  public function call__($name, $args);
  public function __soapcall(string $name, $args, $options = null, $input_headers = null);
  public function __getlastrequest();
  public function __getlastresponse();
  public function __getlastrequestheaders();
  public function __getlastresponseheaders();
  public function __getfunctions();
  public function __gettypes();
  public function __dorequest(string $buf, string $location, string $action, int $version, bool $oneway = false);
  public function __setcookie(string $name, $value = null);
  public function __setlocation($new_location = null);
  public function __setsoapheaders($headers = null);
}

class SoapVar {
  public function __construct($data, $type, string $type_name = "", string $type_namespace = "", string $node_name = "", string $node_namespace = "");
}

class SoapParam {
  public function __construct($data, string $name);
}

class SoapHeader {
  public function __construct(string $ns, string $name, $data = null, bool $mustunderstand = false, $actor = null);
}

class SoapFault extends Exception {

  // Properties
  public $faultcode;
  public $faultcodens;
  public $faultstring;
  public $faultactor;
  public $detail;
  public $_name;
  public $headerfault;

  // Methods
  public function __construct(
    $code,
    $message,
    $actor = null,
    $detail = null,
    $name = null,
    $header = null,
  );
  public function __toString();

}
