<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function use_soap_error_handler($handler = true) { }
function is_soap_fault($fault) { }
function _soap_active_version() { }
class SoapServer {
  public function __construct($wsdl, $options = null) { }
  public function setclass($name, ...) { }
  public function setobject($obj) { }
  public function addfunction($func) { }
  public function getfunctions() { }
  public function handle($request = null) { }
  public function setpersistence($mode) { }
  public function fault($code, $fault, $actor = null, $detail = null, $name = null) { }
  public function addsoapheader($fault) { }
}
class SoapClient {
  public function __construct($wsdl, $options = null) { }
  public function __call($name, $args) { }
  public function __soapcall($name, $args, $options = null, $input_headers = null_variant, &$output_headers = null) { }
  public function __getlastrequest() { }
  public function __getlastresponse() { }
  public function __getlastrequestheaders() { }
  public function __getlastresponseheaders() { }
  public function __getfunctions() { }
  public function __gettypes() { }
  public function __dorequest($buf, $location, $action, $version, $oneway = false) { }
  public function __setcookie($name, $value = null) { }
  public function __setlocation($new_location = null) { }
  public function __setsoapheaders($headers = null_variant) { }
}
class SoapVar {
  public function __construct($data, $type, $type_name = null, $type_namespace = null, $node_name = null, $node_namespace = null) { }
}
class SoapParam {
  public function __construct($data, $name) { }
}
class SoapHeader {
  public function __construct($ns, $name, $data = null, $mustunderstand = false, $actor = null) { }
}
