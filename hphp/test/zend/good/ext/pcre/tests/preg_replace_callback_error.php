<?php
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/
echo "***Testing preg_replace_callback() : error conditions***\n";
//Zero arguments
echo "\n-- Testing preg_replace_callback() function with Zero arguments --\n";
try { var_dump(preg_replace_callback()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Test preg_replace_callback() with one more than the expected number of arguments
echo "\n-- Testing preg_replace_callback() function with more than expected no. of arguments --\n";
ZendGoodExtPcreTestsPregReplaceCallbackError::$replacement = array('zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine');
function integer_word($matches) {

    return ZendGoodExtPcreTestsPregReplaceCallbackError::$replacement[$matches[0]];
}
$regex = '/\d/';
$subject = 'there are 7 words in this sentence.';
$limit = 10;
$extra_arg = 10;
try { var_dump(preg_replace_callback($regex, 'integer_word', $subject, $limit, &$count, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Testing preg_replace_callback() with one less than the expected number of arguments
echo "\n-- Testing preg_replace_callback() function with less than expected no. of arguments --\n";
$regex = '/\d/';
try { var_dump(preg_replace_callback($regex, 'integer word')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";

abstract final class ZendGoodExtPcreTestsPregReplaceCallbackError {
  public static $replacement;
}
