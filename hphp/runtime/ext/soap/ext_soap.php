<?hh

<<__NativeData>>
class SoapServer {

  <<__Native>>
  public function __construct(
    mixed $wsdl,
    darray<string, mixed> $options = darray[],
  ): void;

  /**
   * Exports all methods from specified class.  The object can be made
   *   persistent across request for a given PHP session with the
   *   SoapServer::setPersistence() method.
   *
   * @param string $name - The name of the exported class.
   *
   */
  <<__Native>>
  public function setClass(string $name, ...$argv): void;

  /**
   * This sets a specific object as the handler for SOAP requests, rather than
   *   just a class as in SoapServer::setClass().
   *
   * @param object $obj - The object to handle the requests.
   *
   */
  <<__Native>>
  public function setobject(mixed $obj): void;

  /**
   * Exports one or more functions for remote clients
   *
   * @param mixed $func - To export one function, pass the function name into
   *   this parameter as a string.  To export several functions, pass an array
   *   of function names.  To export all the functions, pass a special constant
   *   SOAP_FUNCTIONS_ALL.  functions must receive all input arguments in the
   *   same order as defined in the WSDL file (They should not receive any
   *   output parameters as arguments) and return one or more values. To return
   *   several values they must return an array with named output parameters.
   *
   */
  <<__Native>>
  public function addFunction(mixed $func): void;

  /**
   * Returns a list of the defined functions in the SoapServer object. This
   *   method returns the list of all functions added by
   *   SoapServer::addFunction() or SoapServer::setClass().
   *
   * @return mixed - An array of the defined functions.
   *
   */
  <<__Native>>
  public function getfunctions(): mixed;

  /**
   * Processes a SOAP request, calls necessary functions, and sends a response
   *   back.
   *
   * @param string $request - The SOAP request. If this argument is omitted,
   *   the request is assumed to be in the raw POST data of the HTTP request.
   *
   */
  <<__Native>>
  public function handle(?string $request = null): void;

  /**
   * This function allows saving data between requests in a PHP session. It
   *   works only with a server that exports functions from a class with
   *   SoapServer::setClass() or SoapServer::setObject().  The persistence
   *   SOAP_PERSISTENCE_SESSION makes only objects of the given class
   *   persistent, but not the class static data. In this case, use $this->bar
   *   instead of self::$bar.
   *
   * @param int $mode - One of the SOAP_PERSISTENCE_XXX constants.
   *   SOAP_PERSISTENCE_REQUEST - persist the object for the duration of a
   *   request.  SOAP_PERSISTENCE_SESSION - persist the object for the duration
   *   of a session.
   *
   */
  <<__Native>>
  public function setpersistence(int $mode): void;

  /**
   * Sends a response to the client of the current request indicating an
   *   error.  This can only be called when handling a request.
   *
   * @param mixed $code - The error code to return
   * @param string $fault - A brief description of the error
   * @param string $actor - A string identifying the actor that caused the
   *   fault.
   * @param mixed $detail - More details of the fault
   * @param string $name - The name of the fault. This can be used to select a
   *   name from a WSDL file.
   *
   */
  <<__Native>>
  public function fault(mixed $code,
                 string $fault,
                 ?string $actor = null,
                 mixed $detail = null,
                 ?string $name = null): void;

  /**
   * Adds a SOAP header to be returned with the response to the current
   *   request.
   *
   * @param object $fault - The header to be returned.
   *
   */
  <<__Native>>
  public function addSoapHeader(mixed $fault): void;
}

<<__NativeData>>
class SoapClient {

  // Clean out different kinds of arrays, recursively, from an input. For
  // __soapcall, we want to pass in PHP arrays but receive Hack arrays, so we
  // invoke cleanArrays twice: once with a PHP array as the base, once with
  // a Hack array as the base.
  private static function cleanArrays(
    mixed $input,
    mixed $base,
    mixed $seen = null,
  ): mixed {
    if ($seen === null) {
      $seen = new stdClass();
      $seen->set = keyset[];
    }
    if (HH\is_any_array($input)) {
      $ret = $base;
      foreach ($input as $k => $v) {
        $ret[$k] = self::cleanArrays($v, $base, $seen);
      }
      return $ret;
    } else if (is_object($input)) {
      $hash = spl_object_hash($input);
      if (array_key_exists($hash, $seen->set)) {
        return $input;
      }
      $seen->set[] = $hash;
      foreach ($input as $k => $v) {
        $input->$k = self::cleanArrays($v, $base, $seen);
      }
      return $input;
    }
    return $input;
  }

  <<__Native>>
  public function __construct(
    mixed $wsdl,
    darray<string, mixed> $options = darray[],
  ): void;

  <<__Native>>
  private function soapcallImpl(
    string $name,
    varray<mixed> $args,
    darray $options = darray[],
    mixed $input_headers = null,
  ): mixed;

  public function call__(mixed $name, mixed $args): mixed {
    return $this->__soapcall($name, $args);
  }

  public function __soapcall(
    string $name,
    varray<mixed> $args,
    darray $options = darray[],
    mixed $input_headers = null,
  ): mixed {
    $args = self::cleanArrays($args, HH\array_mark_legacy(darray[]));
    $ret = $this->soapcallImpl(
      $name,
      varray($args),
      $options,
      $input_headers,
    );
    return self::cleanArrays($ret, HH\array_mark_legacy(darray[]));
  }

  <<__Native>>
  public function __getlastrequest(): mixed;

  <<__Native>>
  public function __getlastresponse(): mixed;

  <<__Native>>
  public function __getlastrequestheaders(): mixed;

  <<__Native>>
  public function __getlastresponseheaders(): mixed;

  <<__Native>>
  public function __getFunctions(): mixed;

  <<__Native>>
  public function __getTypes(): mixed;

  <<__Native>>
  public function __dorequest(string $buf,
                       string $location,
                       string $action,
                       int $version,
                       bool $oneway = false): mixed;

  <<__Native>>
  public function __setcookie(string $name, ?string $value = null): mixed;

  <<__Native>>
  public function __setlocation(?string $new_location = null): mixed;

  <<__Native>>
  public function __setsoapheaders(mixed $headers = null): bool;
}

/**
 * A class representing a variable or object for use with SOAP services.
 *
 */
class SoapVar {

  <<__Native>>
  public function __construct(mixed $data,
                       mixed $type,
                       string $type_name = "",
                       string $type_namespace = "",
                       string $node_name = "",
                       string $node_namespace = ""): void;
}

/**
 * Represents parameter to a SOAP call.
 *
 */
<<__NativeData>>
class SoapParam {

  <<__Native>>
  public function __construct(mixed $data, string $name): void;
}

/**
 * Represents a SOAP header.
 *
 */
<<__NativeData>>
class SoapHeader {

  <<__Native>>
  public function __construct(string $ns,
                       string $name,
                       mixed $data = null,
                       bool $mustunderstand = false,
                       mixed $actor = null): void;
}

/**
 * This function sets whether or not to use the SOAP error handler in the SOAP
 *   server. It will return the previous value. If set to TRUE, details of
 *   errors in a SoapServer application will be sent to the clients. If FALSE,
 *   no information will be sent.
 *
 * @param bool $handler - Set to TRUE to send error details to clients.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function use_soap_error_handler(bool $handler = true): bool;

/**
 * This function is useful to check if the SOAP call failed, but without using
 *   exceptions. To use it, create a SoapClient object with the exceptions
 *   option set to zero or FALSE. In this case, the SOAP method will return a
 *   special SoapFault object which encapsulates the fault details (faultcode,
 *   faultstring, faultactor and faultdetails).  If exceptions is not set then
 *   SOAP call will throw an exception on error. is_soap_fault() checks if the
 *   given parameter is a SoapFault object.
 *
 * @param mixed $fault - The object to test.
 *
 * @return bool - This will return TRUE on error, and FALSE otherwise.
 *
 */
<<__Native>>
function is_soap_fault(mixed $fault): bool;

/**
 * This function returns the soap version that is currently active.
 *
 * @return int - This will return the active soap version.
 *
 */
<<__Native>>
function _soap_active_version(): int;
