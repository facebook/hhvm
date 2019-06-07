<?hh

function fix_err($errmsg, $user_format) {

  if (defined('HHVM_VERSION')) {
    if (!$user_format) {
      $errmsg = preg_replace('/Argument (\d+) passed to ([\w\\\{\}]+)\(\) must be an instance of (\w+), (\w+) given/',
                             '\2() expects parameter \1 to be \3, \4 given',
                             $errmsg);
    }
    $errmsg = preg_replace('/\b__invoke\b/', "{closure}", $errmsg);
    $errmsg = preg_replace('/\bint\b/', "integer", $errmsg);
    $errmsg = preg_replace('/\bdouble\b/', "float", $errmsg);
    $errmsg = preg_replace('/\bstdClass\b/', "object", $errmsg);
    $errmsg = preg_replace('/\bStringable\b/', "object", $errmsg);
    $errmsg = preg_replace('/\bMemFile\b/', "resource", $errmsg);
    $errmsg = preg_replace('/\bbool\b/', "boolean", $errmsg);
    $errmsg = str_replace("be an instance of integer", "be of the type integer", $errmsg);
    $errmsg = str_replace("be an instance of float", "be of the type float", $errmsg);
    $errmsg = str_replace("be an instance of string", "be of the type string", $errmsg);
    $errmsg = str_replace("be an instance of boolean", "be of the type boolean", $errmsg);
    $errmsg = str_replace("be an instance of array", "be of the type array", $errmsg);
  } else {
    $errmsg = preg_replace('/, called in .+/', '', $errmsg);
  }

  return $errmsg;
}
