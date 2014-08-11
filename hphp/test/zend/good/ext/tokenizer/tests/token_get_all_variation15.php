<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Testing token_get_all() with heredoc 'source' string with all different types of token and heredoc string within
 *     <<<EOT - T_START_HEREDOC(371)
 *     EOT - T_END_HEREDOC(372)
*/

echo "*** Testing token_get_all() : with heredoc source string ***\n";

$source = <<<EOT
<?= 
  \$a = 2;
  \$b = 1;
  \$c = <<<EOS
  This is to test 
  heredoc string
EOS;
  echo \$a + \$b;
  function myFunction(\$a)
  {
    var_dump(\$a);
  }
  if(\$b < 10) {
    \$b++;
  }
  else
    \$b--;
  while(\$a > 0) {
    echo "*";
    \$a--;
  }
  myFunction(10);
?>
EOT;
var_dump( token_get_all($source));

echo "Done"
?>
