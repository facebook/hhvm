<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_replace_callback reacts to being passed the wrong type of regex argument
*/
echo "*** Testing preg_replace_callback() : error conditions ***\n";
$regex_array = array('abcdef', //Regex without delimiters
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', array('[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'), '/[a-zA-Z]/'); //Regex string
ZendGoodExtPcreTestsPregReplaceCallbackError1::$replacement = array('zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine');
function integer_word($matches) {
  try {
    return ZendGoodExtPcreTestsPregReplaceCallbackError1::$replacement[$matches[0]];
  } catch (Exception $_) {
    return null;
  }
}
$subject = 'number 1.';
foreach($regex_array as $regex_value) {
    print "\nArg value is $regex_value\n";
    $count = -1;
    var_dump(preg_replace_callback($regex_value, fun('integer_word'), $subject, -1, inout $count));
}

abstract final class ZendGoodExtPcreTestsPregReplaceCallbackError1 {
  public static $replacement;
}
echo "===Done===";
