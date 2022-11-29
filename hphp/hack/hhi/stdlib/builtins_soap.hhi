<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int SOAP_1_1;
const int SOAP_1_2;
const int SOAP_ACTOR_NEXT;
const int SOAP_ACTOR_NONE;
const int SOAP_ACTOR_UNLIMATERECEIVER;
const int SOAP_AUTHENTICATION_BASIC;
const int SOAP_AUTHENTICATION_DIGEST;
const int SOAP_COMPRESSION_ACCEPT;
const int SOAP_COMPRESSION_DEFLATE;
const int SOAP_COMPRESSION_GZIP;
const int SOAP_DOCUMENT;
const int SOAP_ENCODED;
const int SOAP_ENC_ARRAY;
const int SOAP_ENC_OBJECT;
const int SOAP_FUNCTIONS_ALL;
const int SOAP_LITERAL;
const int SOAP_PERSISTENCE_REQUEST;
const int SOAP_PERSISTENCE_SESSION;
const int SOAP_RPC;
const int SOAP_SINGLE_ELEMENT_ARRAYS;
const int SOAP_USE_XSI_ARRAY_TYPE;
const int SOAP_WAIT_ONE_WAY_CALLS;

const string XSD_NAMESPACE;
const string XSD_1999_NAMESPACE;

const int XSD_1999_TIMEINSTANT;
const int XSD_ANYTYPE;
const int XSD_ANYURI;
const int XSD_ANYXML;
const int XSD_BASE64BINARY;
const int XSD_BOOLEAN;
const int XSD_BYTE;
const int XSD_DATE;
const int XSD_DATETIME;
const int XSD_DECIMAL;
const int XSD_DOUBLE;
const int XSD_DURATION;
const int XSD_ENTITIES;
const int XSD_ENTITY;
const int XSD_FLOAT;
const int XSD_GDAY;
const int XSD_GMONTH;
const int XSD_GMONTHDAY;
const int XSD_GYEAR;
const int XSD_GYEARMONTH;
const int XSD_HEXBINARY;
const int XSD_ID;
const int XSD_IDREF;
const int XSD_IDREFS;
const int XSD_INT;
const int XSD_INTEGER;
const int XSD_LANGUAGE;
const int XSD_LONG;
const int XSD_NAME;
const int XSD_NCNAME;
const int XSD_NEGATIVEINTEGER;
const int XSD_NMTOKEN;
const int XSD_NMTOKENS;
const int XSD_NONNEGATIVEINTEGER;
const int XSD_NONPOSITIVEINTEGER;
const int XSD_NORMALIZEDSTRING;
const int XSD_NOTATION;
const int XSD_POSITIVEINTEGER;
const int XSD_QNAME;
const int XSD_SHORT;
const int XSD_STRING;
const int XSD_TIME;
const int XSD_TOKEN;
const int XSD_UNSIGNEDBYTE;
const int XSD_UNSIGNEDINT;
const int XSD_UNSIGNEDLONG;
const int XSD_UNSIGNEDSHORT;

const int WSDL_CACHE_NONE;
const int WSDL_CACHE_MEMORY;
const int WSDL_CACHE_DISK;
const int WSDL_CACHE_BOTH;

<<__PHPStdLib>>
function use_soap_error_handler(
  bool $handler = true,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function is_soap_fault($fault): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function _soap_active_version(): HH\FIXME\MISSING_RETURN_TYPE;

class SoapServer {
  public function __construct($wsdl, $options = null);
  public function setClass(
    string $name,
    ...$args
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setobject($obj): HH\FIXME\MISSING_RETURN_TYPE;
  public function addFunction($func): HH\FIXME\MISSING_RETURN_TYPE;
  public function getfunctions(): HH\FIXME\MISSING_RETURN_TYPE;
  public function handle($request = null): HH\FIXME\MISSING_RETURN_TYPE;
  public function setpersistence(int $mode): HH\FIXME\MISSING_RETURN_TYPE;
  public function fault(
    $code,
    string $fault,
    $actor = null,
    $detail = null,
    $name = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function addSoapHeader($fault): HH\FIXME\MISSING_RETURN_TYPE;
}

class SoapClient {
  public function __construct($wsdl, $options = null);
  public function call__($name, $args): HH\FIXME\MISSING_RETURN_TYPE;
  public function __soapcall(
    string $name,
    $args,
    $options = null,
    $input_headers = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function __getlastrequest(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __getlastresponse(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __getlastrequestheaders(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __getlastresponseheaders(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __getFunctions(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __getTypes(): HH\FIXME\MISSING_RETURN_TYPE;
  public function __dorequest(
    string $buf,
    string $location,
    string $action,
    int $version,
    bool $oneway = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function __setcookie(
    string $name,
    $value = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function __setlocation(
    $new_location = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function __setsoapheaders(
    $headers = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}

class SoapVar {
  public function __construct(
    $data,
    $type,
    string $type_name = "",
    string $type_namespace = "",
    string $node_name = "",
    string $node_namespace = "",
  );
}

class SoapParam {
  public function __construct($data, string $name);
}

class SoapHeader {
  public function __construct(
    string $ns,
    string $name,
    $data = null,
    bool $mustunderstand = false,
    $actor = null,
  );
}

class SoapFault extends Exception {

  // Properties
  public HH\FIXME\MISSING_PROP_TYPE $faultcode;
  public HH\FIXME\MISSING_PROP_TYPE $faultcodens;
  public HH\FIXME\MISSING_PROP_TYPE $faultstring;
  public HH\FIXME\MISSING_PROP_TYPE $faultactor;
  public HH\FIXME\MISSING_PROP_TYPE $detail;
  public HH\FIXME\MISSING_PROP_TYPE $_name;
  public HH\FIXME\MISSING_PROP_TYPE $headerfault;

  // Methods
  public function __construct(
    $code,
    $message,
    $actor = null,
    $detail = null,
    $name = null,
    $header = null,
  );
  public function __toString(): string;

}
