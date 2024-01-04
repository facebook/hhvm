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
   * Test how mb_ereg() matches special characters for $pattern
   */

  echo "*** Testing mb_ereg() : usage variations ***\n";

  if(mb_regex_encoding('utf-8') == true) {
  	echo "Regex encoding set to utf-8\n";
  } else {
  	echo "Could not set regex encoding to utf-8\n";
  }

  $regex_char = dict['\w+' => b'\w+',
                       '\W+' => b'\W+',
                       '\s+' => b'\s+',
                       '\S+' => b'\S+',
                       '\d+' => b'\d+',
                       '\D+' => b'\D+',
                       '\b' =>  b'\b',
                       '\B' =>  b'\B'];

  $string_ascii = b'This is an English string. 0123456789.';
  $string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

  $regs_ascii = null;
  $regs_mb = null;
  foreach ($regex_char as $displayChar => $char) {
  	echo "\n--** Pattern is: $displayChar **--\n";
  	if ($regs_ascii || $regs_mb) {
  		$regs_ascii = null;
  		$regs_mb = null;
  	}
  	echo "-- ASCII String: --\n";
  	var_dump(mb_ereg($char, $string_ascii, inout $regs_ascii));
  	base64_encode_var_dump($regs_ascii);

  	echo "-- Multibyte String: --\n";
  	var_dump(mb_ereg($char, $string_mb, inout $regs_mb));
  	base64_encode_var_dump($regs_mb);

  }

  echo "Done";
}
