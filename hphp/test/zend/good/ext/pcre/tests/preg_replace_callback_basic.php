<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/
/*
* Basic test for preg_replace_callback
*/
function integer_word($matches) :mixed{
  // Maps from key values (0-9) to corresponding key written in words.
  $replacement = vec['zero', 'one', 'two', 'three', 'four',
                       'five', 'six', 'seven', 'eight', 'nine'];
  // All examples will be looking for an integer value, if one is found it
  // will be stored in $matches[0] which corresponds to a key in the
  // $replacements array.
  return $replacement[\HH\array_key_cast($matches[0])];
}

<<__EntryPoint>> function main(): void {
$count = -1;
$subject1 = 'there are 7 words in this sentence.';
$new_subject1 = preg_replace_callback('/\d/', integer_word<>, $subject1, -1, inout $count);
print "$new_subject1 \n";
$subject2 = '1 2 3 4 is now written in words';
$new_subject2 = preg_replace_callback('/\d/', integer_word<>, $subject2, 3, inout $count); //limits to three replacements
print "$new_subject2 \n";
$subject3 = 'there are no numbers in this string';
$new_subject3 = preg_replace_callback('/\d/', integer_word<>, $subject3, 5, inout $count); //limites to five replacements and counts the number of replacements made ands stores in $count variable
print "$new_subject3 \n";
print $count;
}
