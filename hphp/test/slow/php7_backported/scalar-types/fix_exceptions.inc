<?hh

function fix_exceptions() {
set_error_handler(function (int $errno, string $errmsg, string $file, int $line) {
  $errnames = dict[
    E_NOTICE => 'E_NOTICE',
    E_WARNING => 'E_WARNING',
    E_RECOVERABLE_ERROR => 'E_RECOVERABLE_ERROR'
  ];
    if ($errno == E_NOTICE) {
      echo "$errnames[$errno]: $errmsg on line $line\n";
      return true;
    }

    $errmsg = str_replace("__invoke", "{closure}", $errmsg);
    $errmsg = str_replace("int", "integer", $errmsg);
    $errmsg = str_replace("double", "float", $errmsg);
    $errmsg = str_replace("stdClass", "object", $errmsg);
    $errmsg = str_replace("Stringable", "object", $errmsg);
    $errmsg = str_replace("MemFile", "resource", $errmsg);
    $errmsg = str_replace("bool", "boolean", $errmsg);
    $errmsg = str_replace("be an instance of integer", "be of the type integer", $errmsg);
    $errmsg = str_replace("be an instance of float", "be of the type float", $errmsg);
    $errmsg = str_replace("be an instance of string", "be of the type string", $errmsg);
    $errmsg = str_replace("be an instance of boolean", "be of the type boolean", $errmsg);
    $errmsg = str_replace("Value returned from closure must be of type", "Return value of {closure}() must be of the type", $errmsg);
    throw new TypeError("$errmsg, called in $file on line $line");
});
}
