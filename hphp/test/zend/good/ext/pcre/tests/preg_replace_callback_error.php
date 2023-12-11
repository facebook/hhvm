<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/

function integer_word($matches) :mixed{
  // Maps from key values (0-9) to corresponding key written in words.
  $replacement = vec['zero', 'one', 'two', 'three', 'four',
                       'five', 'six', 'seven', 'eight', 'nine'];
  return $replacement[$matches[0]];
}




<<__EntryPoint>> function main(): void {
echo "***Testing preg_replace_callback() : error conditions***\n";
//Zero arguments
echo "\n-- Testing preg_replace_callback() function with Zero arguments --\n";
try { var_dump(preg_replace_callback()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Test preg_replace_callback() with one more than the expected number of arguments
echo "\n-- Testing preg_replace_callback() function with more than expected no. of arguments --\n";
$regex = '/\d/';
$subject = 'there are 7 words in this sentence.';
$limit = 10;
$extra_arg = 10;
$count = -1;
try { var_dump(preg_replace_callback($regex, integer_word<>, $subject, $limit, inout $count, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Testing preg_replace_callback() with one less than the expected number of arguments
echo "\n-- Testing preg_replace_callback() function with less than expected no. of arguments --\n";
$regex = '/\d/';
try { var_dump(preg_replace_callback($regex, integer_word<>)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
