<?hh


function utf8_strtolower($str): string {
  return mb_strtolower($str, 'utf-8');
}

function normalize_email($email, $strip_slashes = false): string {
  $ret_email = trim(utf8_strtolower($email), " \t\r\n\0\x0B.");
  if ($strip_slashes) {
    $ret_email = stripslashes($ret_email);
  }

  return $ret_email;
}

var_dump(normalize_email('xxxxxxxxxx@yyyyy.com'.str_repeat(' ',1518)));
