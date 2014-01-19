<?

function main() {
  function e($errno, $errstr, $errfile, $errline, $errcontext='',
             $errtrace = array()) {
    $str = 'PHP: ';
    $str .= $errstr;
    $str .= " (in $errfile on line $errline)";
    SandboxErrorToFatalConverter::processError($errno, $errstr, $str, $errfile);
    throw new C;
  }
  set_error_handler("e");
  function g() { }
  function g() { } // fatal error
}

main();
