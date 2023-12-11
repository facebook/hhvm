<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/

function integer_word($matches) :mixed{
  // Maps from key values (0-9) to corresponding key written in words.
  $replacement = vec['zero', 'one', 'two', 'three', 'four',
                       'five', 'six', 'seven', 'eight', 'nine'];
  try {
    return $replacement[$matches[0]];
  } catch (Exception $_) {
    return null;
  }
}




<<__EntryPoint>> function main(): void {
/*
* Testing how preg_replace_callback reacts to being passed the wrong type of regex argument
*/
echo "*** Testing preg_replace_callback() : error conditions ***\n";
$regex_array = vec['abcdef', //Regex without delimiters
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', vec['[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'], '/[a-zA-Z]/']; //Regex string
$subject = 'number 1.';
foreach($regex_array as $regex_value) {
    $text = HH\is_any_array($regex_value) ? 'Array' : $regex_value;
    print "\nArg value is $text\n";
    $count = -1;
    var_dump(preg_replace_callback($regex_value, integer_word<>, $subject, -1, inout $count));
}

echo "===Done===";
}
