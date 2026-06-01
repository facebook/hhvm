<?hh
/**
 * replicate a var dump of an array but outputted string values are base64 encoded
 *
 * @param array $regs
 */
function base64_encode_var_dump($regs) :mixed{
	if ($regs) {
		echo "array(" . count($regs) . ") {\n";
		foreach ($regs as $key => $value) {
			echo "  [$key]=>\n  ";
			if (is_string($value)) {
				var_dump(base64_encode($value));
			} else {
				var_dump($value);
			}
		}
		echo "}\n";
	} else {
		echo "NULL\n";
	}
}
<<__EntryPoint>>
function main_entry(): void {
  /* Prototype  : int mb_ereg(string $pattern, string $string [, array $registers])
   * Description: Regular expression match for multibyte string
   * Source code: ext/mbstring/php_mbregex.c
   */

  /*
   * test that mb_ereg can match correctly when passed different character classes.
   */

  echo "*** Testing mb_ereg() : variation ***\n";


  mb_regex_encoding('utf-8'); // have to set otherwise won't match $mb properly
  $mb = base64_decode('5pel5pys6Kqe');
  $character_classes = dict['aB1'    => '[[:alnum:]]+', /*1*/
                              'aBcD'   => '[[:alpha:]]+',
                              'ab/='   => '[[:ascii:]]+',
                              " \t"    => '[[:blank:]]+',
                              '234'    => '[[:digit:]]+', /*5*/
                              "$mb"    => '[[:graph:]]+',
                              'fjds'   => '[[:lower:]]+',
                              "$mb\t"  => '[[:print:]]+',
                              '.!"*@'  => '[[:punct:]]+',
                              "\t"     => '[[:space:]]+', /*10*/
                              'IDSJV'  => '[[:upper:]]+',
                              '3b5D'   => '[[:xdigit:]]+']; /*12*/

  $iterator = 1;
  $regs = null;
  foreach($character_classes as $string => $pattern) {
  	if (is_array($regs)) {
  		$regs = null;
  	}
  	// make sure any multibyte output is in base 64
  	echo "\n-- Iteration $iterator --\n";
  	var_dump(mb_ereg($pattern, (string)$string, inout $regs));
  	base64_encode_var_dump($regs);
  	$iterator++;
  }

  echo "Done";
}
