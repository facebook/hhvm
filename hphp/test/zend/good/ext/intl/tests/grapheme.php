<?hh

/*
 * Test grapheme functions (procedural only)
 */

function ut_main()
:mixed{
	$res_str = '';

	$char_a_diaeresis = "\xC3\xA4";	// 'LATIN SMALL LETTER A WITH DIAERESIS' (U+00E4)
	$char_a_ring = "\xC3\xA5";		// 'LATIN SMALL LETTER A WITH RING ABOVE' (U+00E5)
	$char_o_diaeresis = "\xC3\xB6";    // 'LATIN SMALL LETTER O WITH DIAERESIS' (U+00F6)
	$char_O_diaeresis = "\xC3\x96";    // 'LATIN CAPITAL LETTER O WITH DIAERESIS' (U+00D6)

	$char_angstrom_sign = "\xE2\x84\xAB"; // 'ANGSTROM SIGN' (U+212B)
	$char_A_ring = "\xC3\x85";	// 'LATIN CAPITAL LETTER A WITH RING ABOVE' (U+00C5)

	$char_ohm_sign = "\xE2\x84\xA6";	// 'OHM SIGN' (U+2126)
	$char_omega = "\xCE\xA9";  // 'GREEK CAPITAL LETTER OMEGA' (U+03A9)

	$char_combining_ring_above = "\xCC\x8A";  // 'COMBINING RING ABOVE' (U+030A)

	$char_fi_ligature = "\xEF\xAC\x81";  // 'LATIN SMALL LIGATURE FI' (U+FB01)

	$char_long_s_dot = "\xE1\xBA\x9B";	// 'LATIN SMALL LETTER LONG S WITH DOT ABOVE' (U+1E9B)

	// the word 'hindi' using Devanagari characters:
	$hindi = "\xe0\xa4\xb9\xe0\xa4\xbf\xe0\xa4\xa8\xe0\xa5\x8d\xe0\xa4\xa6\xe0\xa5\x80";

	$char_a_ring_nfd = "a\xCC\x8A";
	$char_A_ring_nfd = "A\xCC\x8A";
	$char_o_diaeresis_nfd = "o\xCC\x88";
	$char_O_diaeresis_nfd = "O\xCC\x88";
	$char_diaeresis = "\xCC\x88";

	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_strlen($string) {}' . "\n\n";


	$res_str .= "\"hindi\" in devanagari strlen " . grapheme_strlen($hindi) . "\n";
	$res_str .= "\"ab\" + \"hindi\" + \"cde\" strlen " . grapheme_strlen('ab' . $hindi . 'cde') . "\n";
	$res_str .= "\"\" strlen " . grapheme_strlen("") . "\n";
	$res_str .= "char_a_ring_nfd strlen " . grapheme_strlen($char_a_ring_nfd) . "\n";
	$res_str .= "char_a_ring_nfd + \"bc\" strlen " . grapheme_strlen($char_a_ring_nfd . 'bc') . "\n";
	$res_str .= "\"abc\" strlen " . grapheme_strlen('abc') . "\n";


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_strpos($haystack, $needle, $offset = 0) {}' . "\n\n";

	$tests = vec[
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2 ],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 1 ],
		vec[ "abc", $char_a_ring_nfd, "false" ],
		vec[ $char_a_ring_nfd . "bc", "a", "false" ],
		vec[ "abc", "d", "false" ],
		vec[ "abc", "c", 2 ],
		vec[ "abc", "b", 1 ],
		vec[ "abc", "a", 0 ],
		vec[ "abc", "a", 0, 0 ],
		vec[ "abc", "a", 1, "false" ],
		vec[ "ababc", "a", 1, 2 ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "op", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "opq", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 2 ],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 1 ],
		vec[ "abc", $char_a_ring_nfd . "bc", "false" ],
		vec[ $char_a_ring_nfd . "bc", "abcdefg", "false" ],
		vec[ "abc", "defghijklmnopq", "false" ],
		vec[ "abc", "ab", 0 ],
		vec[ "abc", "bc", 1 ],
		vec[ "abc", "abc", 0 ],
		vec[ "abc", "abcd", "false" ],
		vec[ "abc", "ab", 0, 0 ],
		vec[ "abc", "abc", 0, 0 ],
		vec[ "abc", "abc", 1, "false" ],
		vec[ "ababc", "ab", 1, 2 ],
		vec[ "ababc", "abc", 1, 2 ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_a_ring_nfd . "bc", "o" . $char_a_ring_nfd . "bc", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "bc" . $char_a_ring_nfd, 2, 3 ],
	];

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strpos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strpos($test[0], $test[1]);
		}
		else {
			$res_str .= " from ".$test[2];
			$result = grapheme_strpos($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= $result;
		}
		$res_str .= " == " . $test[count($test)-1] . check_result($result, $test[count($test)-1]) . "\n";
	}

	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_stripos($haystack, $needle, $offset = 0) {}' . "\n\n";

	$tests = vec[
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "O", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_O_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd, 2 ],
		vec[ "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 1 ],
		vec[ "Abc", $char_a_ring_nfd, "false" ],
		vec[ $char_a_ring_nfd . "bc", "A", "false" ],
		vec[ "abc", "D", "false" ],
		vec[ "abC", "c", 2 ],
		vec[ "abc", "B", 1 ],
		vec[ "Abc", "a", 0 ],
		vec[ "abc", "A", 0, 0 ],
		vec[ "Abc", "a", 1, "false" ],
		vec[ "ababc", "A", 1, 2 ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "oP", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "opQ", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bC" . $char_o_diaeresis_nfd, $char_O_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "Bc", $char_A_ring_nfd . "bc", 2 ],
		vec[ "a" . $char_a_ring_nfd . "BC", $char_a_ring_nfd . "bc", 1 ],
		vec[ "abc", $char_a_ring_nfd . "BC", "false" ],
		vec[ $char_a_ring_nfd . "BC", "aBCdefg", "false" ],
		vec[ "aBC", "Defghijklmnopq", "false" ],
		vec[ "abC", "Ab", 0 ],
		vec[ "aBC", "bc", 1 ],
		vec[ "abC", "Abc", 0 ],
		vec[ "abC", "aBcd", "false" ],
		vec[ "ABc", "ab", 0, 0 ],
		vec[ "aBc", "abC", 0, 0 ],
		vec[ "abc", "aBc", 1, "false" ],
		vec[ "ABabc", "AB", 1, 2 ],
		vec[ "abaBc", "aBc", 1, 2 ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_A_ring_nfd . "bC", "O" . $char_a_ring_nfd . "bC", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bC" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "Bc" . $char_a_ring_nfd, 2, 3 ],
	];

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_stripos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_stripos($test[0], $test[1]);
		}
		else {
			$res_str .= " from ".$test[2];
			$result = grapheme_stripos($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= $result;
		}
		$res_str .= " == " . $test[count($test)-1] . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_strrpos($haystack, $needle, $offset = 0) {}' . "\n\n";


	$tests = vec[
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2 ],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 1 ],
		vec[ "abc", $char_a_ring_nfd, "false" ],
		vec[ $char_a_ring_nfd . "bc", "a", "false" ],
		vec[ "abc", "d", "false" ],
		vec[ "abc", "c", 2 ],
		vec[ "abc", "b", 1 ],
		vec[ "abc", "a", 0 ],
		vec[ "abc", "a", 0, 0 ],
		vec[ "abc", "a", 1, "false" ],
		vec[ "ababc", "a", 1, 2 ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "op", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "opq", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 2 ],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 1 ],
		vec[ "abc", $char_a_ring_nfd . "bc", "false" ],
		vec[ $char_a_ring_nfd . "bc", "abcdefg", "false" ],
		vec[ "abc", "defghijklmnopq", "false" ],
		vec[ "abc", "ab", 0 ],
		vec[ "abc", "bc", 1 ],
		vec[ "abc", "abc", 0 ],
		vec[ "abc", "abcd", "false" ],
		vec[ "abc", "ab", 0, 0 ],
		vec[ "abc", "abc", 0, 0 ],
		vec[ "abc", "abc", 1, "false" ],
		vec[ "ababc", "ab", 1, 2 ],
		vec[ "ababc", "abc", 1, 2 ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_a_ring_nfd . "bc", "o" . $char_a_ring_nfd . "bc", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "bc" . $char_a_ring_nfd, 2, 3 ],
	];

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strrpos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strrpos($test[0], $test[1]);
		}
		else {
			$res_str .= " from ".$test[2];
			$result = grapheme_strrpos($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= $result;
		}
		$res_str .= " == " . $test[count($test)-1] .  check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_strripos($haystack, $needle, $offset = 0) {}' . "\n\n";

	$tests = vec[
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "O", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_O_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd, 2 ],
		vec[ "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 1 ],
		vec[ "Abc", $char_a_ring_nfd, "false" ],
		vec[ $char_a_ring_nfd . "bc", "A", "false" ],
		vec[ "abc", "D", "false" ],
		vec[ "abC", "c", 2 ],
		vec[ "abc", "B", 1 ],
		vec[ "Abc", "a", 0 ],
		vec[ "abc", "A", 0, 0 ],
		vec[ "Abc", "a", 1, "false" ],
		vec[ "ababc", "A", 1, 2 ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "oP", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "opQ", 5 ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bC" . $char_o_diaeresis_nfd, $char_O_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "Bc", $char_A_ring_nfd . "bc", 2 ],
		vec[ "a" . $char_a_ring_nfd . "BC", $char_a_ring_nfd . "bc", 1 ],
		vec[ "abc", $char_a_ring_nfd . "BC", "false" ],
		vec[ $char_a_ring_nfd . "BC", "aBCdefg", "false" ],
		vec[ "aBC", "Defghijklmnopq", "false" ],
		vec[ "abC", "Ab", 0 ],
		vec[ "aBC", "bc", 1 ],
		vec[ "abC", "Abc", 0 ],
		vec[ "abC", "aBcd", "false" ],
		vec[ "ABc", "ab", 0, 0 ],
		vec[ "aBc", "abC", 0, 0 ],
		vec[ "abc", "aBc", 1, "false" ],
		vec[ "ABabc", "AB", 1, 2 ],
		vec[ "abaBc", "aBc", 1, 2 ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_A_ring_nfd . "bC", "O" . $char_a_ring_nfd . "bC", 2, 6 ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bC" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "Bc" . $char_a_ring_nfd, 2, 3 ],
	];

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strripos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strripos($test[0], $test[1]);
		}
		else {
			$res_str .= " from ".$test[2];
			$result = grapheme_strripos($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= $result;
		}
		$res_str .= " == " . $test[count($test)-1] . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_substr($string, $start, $length = -1) {}' . "\n\n";

	$tests = vec[

		vec[ "abc", 3, "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, 5, "false" ],
		vec[ "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", 2, $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ],
		vec[ $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bc", 2, "a" . $char_A_ring_nfd . "bc" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", 5, "O" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, 5, "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_O_diaeresis_nfd, 4, $char_O_diaeresis_nfd ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", 2, $char_a_ring_nfd . "bc" ],
		vec[ "a" . $char_A_ring_nfd . "bc", 1, $char_A_ring_nfd . "bc" ],
		vec[ "Abc", -5, "false" ],
		vec[ $char_a_ring_nfd . "bc", 3, "false" ],
		vec[ "abc", 4, "false" ],
		vec[ "abC", 2, "C" ],
		vec[ "abc", 1, "bc" ],
		vec[ "Abc", 1, 1, "b" ],
		vec[ "abc", 0, 2, "ab" ],
		vec[ "Abc", -4, 1, "false" ],
		vec[ "ababc", 1, 2, "ba" ],
		vec[ "ababc", 0, 10, "ababc" ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, 10 , "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -1, "Op" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -2, "O" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -3, "" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -4, "false" ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -1, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Op" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -2, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -3, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -4, "a" . $char_a_ring_nfd . "bc" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -5, "a" . $char_a_ring_nfd . "b" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -6, "a" . $char_a_ring_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -7, "a" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -8, "" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -9, "false" ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -7, $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -6, "bc" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -5, "c" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -4, $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -3, "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -2, "pq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -1, "q" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -999, "false" ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 8, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 7, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Op" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 6, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 5, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 4, "a" . $char_a_ring_nfd . "bc" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 3, "a" . $char_a_ring_nfd . "b" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 2, "a" . $char_a_ring_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 1, "a" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 0, "" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -999, "false" ],

		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -1, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Op" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -2, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -3, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -4, "a" . $char_a_ring_nfd . "bc" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -5, "a" . $char_a_ring_nfd . "b" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -6, "a" . $char_a_ring_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -7, "a" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -8, "" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -9, "false" ],

	];

	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "substring of \"$arg0\" from \"".$test[1]."\" - grapheme_substr";
		if ( 3 == count( $test ) ) {
			$result = grapheme_substr($test[0], $test[1]);
		}
		else {
			$res_str .= " with length ".(string)$test[2];
			$result = grapheme_substr($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= urlencode($result);
		}
		$res_str .= " == " . urlencode($test[count($test)-1]) . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_strstr($haystack, $needle, $before_needle = FALSE) {}' . "\n\n";

	$tests = vec[
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", "o" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd, $char_o_diaeresis_nfd ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, $char_a_ring_nfd . "bc"],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, $char_a_ring_nfd . "bc"],
		vec[ "abc", $char_a_ring_nfd, "false" ],
		vec[ $char_a_ring_nfd . "bc", "a", "false" ],
		vec[ "abc", "d", "false" ],
		vec[ "abc", "c", "c" ],
		vec[ "abc", "b", "bc" ],
		vec[ "abc", "a", "abc" ],
		vec[ "abc", "ab", "abc" ],
		vec[ "abc", "abc", "abc" ],
		vec[ "abc", "bc", "bc" ],
		vec[ "abc", "a", FALSE, "abc" ],
		vec[ "abc", "a", TRUE, "" ],
		vec[ "abc", "b", TRUE, "a" ],
		vec[ "abc", "c", TRUE, "ab" ],
		vec[ "ababc", "bab", TRUE, "a" ],
		vec[ "ababc", "abc", TRUE, "ab" ],
		vec[ "ababc", "abc", FALSE, "abc" ],

		vec[ "ab" . $char_a_ring_nfd . "c", "d", "false" ],
		vec[ "bc" . $char_a_ring_nfd . "a", "a", "a" ],
		vec[ "a" . $char_a_ring_nfd . "bc", "b", "bc" ],
		vec[ $char_a_ring_nfd . "bc", "a", "false" ],
		vec[ $char_a_ring_nfd . "abc", "ab", "abc" ],
		vec[ "abc" . $char_a_ring_nfd, "abc", "abc" . $char_a_ring_nfd],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc" ],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, FALSE, $char_a_ring_nfd . "bc"],
		vec[ "a" . $char_a_ring_nfd . "bc", "a", TRUE, "" ],
		vec[ $char_a_ring_nfd . "abc", "b", TRUE, $char_a_ring_nfd . "a" ],
		vec[ "ab" . $char_a_ring_nfd . "c", "c", TRUE, "ab" . $char_a_ring_nfd ],
		vec[ "aba" . $char_a_ring_nfd . "bc", "ba" . $char_a_ring_nfd . "b", TRUE, "a" ],
		vec[ "ababc" . $char_a_ring_nfd, "abc" . $char_a_ring_nfd, TRUE, "ab" ],
		vec[ "abab" . $char_a_ring_nfd . "c", "ab" . $char_a_ring_nfd . "c", FALSE, "ab" . $char_a_ring_nfd . "c" ],

	];

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strstr";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strstr($test[0], $test[1]);
		}
		else {
			$res_str .= " before flag is " . ( $test[2] ? "TRUE" : "FALSE" );
			$result = grapheme_strstr($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= urlencode($result);
		}
		$res_str .= " == " . urlencode($test[count($test)-1]) . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_stristr($haystack, $needle, $before_needle = FALSE) {}' . "\n\n";

	$tests = vec[
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_O_diaeresis_nfd, $char_o_diaeresis_nfd ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", "O" ],
		vec[ "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ],
		vec[ $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, $char_a_ring_nfd . "bc"],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd, $char_a_ring_nfd . "bc"],
		vec[ "abc", $char_a_ring_nfd, "false" ],
		vec[ $char_a_ring_nfd . "bc", "A", "false" ],
		vec[ "abc", "d", "false" ],
		vec[ "abc", "C", "c" ],
		vec[ "aBc", "b", "Bc" ],
		vec[ "abc", "A", "abc" ],
		vec[ "abC", "ab", "abC" ],
		vec[ "abc", "aBc", "abc" ],
		vec[ "abC", "bc", "bC" ],
		vec[ "abc", "A", FALSE, "abc" ],
		vec[ "abc", "a", TRUE, "" ],
		vec[ "aBc", "b", TRUE, "a" ],
		vec[ "abc", "C", TRUE, "ab" ],
		vec[ "aBabc", "bab", TRUE, "a" ],
		vec[ "ababc", "aBc", TRUE, "ab" ],
		vec[ "ababc", "abC", FALSE, "abc" ],

		vec[ "ab" . $char_a_ring_nfd . "c", "d", "false" ],
		vec[ "bc" . $char_a_ring_nfd . "A", "a", "A" ],
		vec[ "a" . $char_a_ring_nfd . "bc", "B", "bc" ],
		vec[ $char_A_ring_nfd . "bc", "a", "false" ],
		vec[ $char_a_ring_nfd . "abc", "Ab", "abc" ],
		vec[ "abc" . $char_A_ring_nfd, "abc", "abc" . $char_A_ring_nfd],
		vec[ "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd . "bc", $char_a_ring_nfd . "bc" ],
		vec[ "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, FALSE, $char_A_ring_nfd . "bc" ],
		vec[ "a" . $char_a_ring_nfd . "bc", "A", TRUE, "" ],
		vec[ $char_a_ring_nfd . "aBc", "b", TRUE, $char_a_ring_nfd . "a" ],
		vec[ "ab" . $char_a_ring_nfd . "c", "C", TRUE, "ab" . $char_a_ring_nfd ],
		vec[ "aba" . $char_A_ring_nfd . "bc", "ba" . $char_a_ring_nfd . "b", TRUE, "a" ],
		vec[ "ababc" . $char_a_ring_nfd, "aBc" . $char_A_ring_nfd, TRUE, "ab" ],
		vec[ "abAB" . $char_A_ring_nfd . "c", "ab" . $char_a_ring_nfd . "c", FALSE, "AB" . $char_A_ring_nfd . "c" ],

	];

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_stristr";
		if ( 3 == count( $test ) ) {
			$result = grapheme_stristr($test[0], $test[1]);
		}
		else {
			$res_str .= " before flag is " . ( $test[2] ? "TRUE" : "FALSE" );
			$result = grapheme_stristr($test[0], $test[1], $test[2]);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= urlencode($result);
		}
		$res_str .= " == " . urlencode($test[count($test)-1]) . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_extract($haystack, $size, $extract_type = GRAPHEME_EXTR_COUNT, $start = 0[, $next])' . "\n\n";

	$tests = vec[
		// haystack, count, [[offset], [next]], result
		vec[ "abc", 3, "abc" ],
		vec[ "abc", 2, "ab" ],
		vec[ "abc", 1, "a" ],
		vec[ "abc", 0, "" ],
		vec[ "abc", 1, 0, "a" ],
		vec[ "abc", 1, 1, "b" ],
		vec[ "abc", 1, 2, "c" ],
		vec[ "abc", 0, 2, "" ],

		vec[ "abc", 3, 0, 3, "abc" ],
		vec[ "abc", 2, 0, 2, "ab" ],
		vec[ "abc", 1, 0, 1, "a" ],
		vec[ "abc", 0, 0, 0, "" ],
		vec[ "abc", 1, 0, 1, "a" ],
		vec[ "abc", 1, 1, 2, "b" ],
		vec[ "abc", 1, 2, 3, "c" ],
		vec[ "abc", 0, 2, 2, "" ],
        vec[ "http://news.bbc.co.uk/2/hi/middle_east/7831588.stm", 48, 48 , 50 , "tm" ],

		vec[ $char_a_ring_nfd . "bc", 3, $char_a_ring_nfd . "bc" ],
		vec[ $char_a_ring_nfd . "bc", 2, $char_a_ring_nfd . "b" ],
		vec[ $char_a_ring_nfd . "bc", 1, $char_a_ring_nfd . "" ],
		vec[ $char_a_ring_nfd . "bc", 3, 0, 5, $char_a_ring_nfd . "bc" ],
		vec[ $char_a_ring_nfd . "bc", 2, 0, 4, $char_a_ring_nfd . "b" ],
		vec[ $char_a_ring_nfd . "bc", 1, 0, 3, $char_a_ring_nfd . "" ],
		vec[ $char_a_ring_nfd . "bcde", 2, 3, 5, "bc" ],
		vec[ $char_a_ring_nfd . "bcde", 2, 4, 6, "cd" ],
		vec[ $char_a_ring_nfd . "bcde" . $char_a_ring_nfd . "f", 4, 5, 11, "de" . $char_a_ring_nfd . "f" ],

		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, $char_a_ring_nfd . $char_o_diaeresis_nfd ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 1, $char_a_ring_nfd . "" ],

		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 0, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 2, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 3, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 4, $char_diaeresis],

		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 0, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 2, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 3, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 4, $char_diaeresis . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 7, $char_diaeresis . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 8, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 10, $char_diaeresis],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 11, "false"],

	];

	$next = -1;
	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "extract from \"$arg0\" \"".$test[1]."\" graphemes - grapheme_extract";
		if ( 3 == count( $test ) ) {
          $tmp = null;
          $result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_COUNT, 0, inout $tmp);
		}
		else if ( 4 == count ( $test ) ) {
			$res_str .= " starting at byte position ".(string)$test[2];
            $tmp = null;
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_COUNT, $test[2], inout $tmp);
		}
		else {
			$res_str .= " starting at byte position ".(string)$test[2]." with \$next";
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_COUNT, $test[2], inout $next);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= urlencode($result);
		}
		$res_str .= " == " . urlencode($test[count($test)-1]) . check_result($result, $test[count($test)-1]);
		if ( 5 == count ( $test ) ) {
			$res_str .= " \$next=$next == ".$test[3]." ";
			if ( $next != $test[3] ) {
				$res_str .= "***FAILED***";
			}
		}
		$res_str .= "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_extract($haystack, $size, $extract_type = GRAPHEME_EXTR_MAXBYTES, $start = 0)' . "\n\n";

	$tests = vec[
		vec[ "abc", 3, "abc" ],
		vec[ "abc", 2, "ab" ],
		vec[ "abc", 1, "a" ],
		vec[ "abc", 0, "" ],
		vec[ $char_a_ring_nfd . "bc", 5, $char_a_ring_nfd . "bc" ],
		vec[ $char_a_ring_nfd . "bc", 4, $char_a_ring_nfd . "b" ],
		vec[ $char_a_ring_nfd . "bc", 1, "" ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 9, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 10, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 11, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, $char_a_ring_nfd . $char_o_diaeresis_nfd ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 3, $char_a_ring_nfd . "" ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 4, $char_a_ring_nfd . "" ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 5, $char_a_ring_nfd . "" ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 6, $char_a_ring_nfd . $char_o_diaeresis_nfd  ],
		vec[ $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 7, $char_a_ring_nfd . $char_o_diaeresis_nfd . "c" ],

		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 0, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 2, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 3, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 4, $char_diaeresis],

		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, 0, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, 2, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, 3, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 5, 4, $char_diaeresis . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 5, 7, $char_diaeresis . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 8, $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 10, $char_diaeresis],
		vec[ $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 11, "false"],

	];

	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "extract from \"$arg0\" \"".$test[1]."\" graphemes - grapheme_extract GRAPHEME_EXTR_MAXBYTES";
		if ( 3 == count( $test ) ) {
            $tmp = null;
            $result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXBYTES, 0, inout $tmp);
		}
		else {
			$res_str .= " starting at byte position ".$test[2];
            $tmp = null;
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXBYTES, $test[2], inout $tmp);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= urlencode($result);
		}
		$res_str .= " == " . urlencode($test[count($test)-1]) . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_extract($haystack, $size, $extract_type = GRAPHEME_EXTR_MAXCHARS, $start = 0)' . "\n\n";

	$tests = vec[
		vec[ "abc", 3, "abc" ],
		vec[ "abc", 2, "ab" ],
		vec[ "abc", 1, "a" ],
		vec[ "abc", 0, "" ],
		vec[ "abc" . $char_o_diaeresis_nfd, 0, "" ],
		vec[ "abc" . $char_o_diaeresis_nfd, 1, "a" ],
		vec[ "abc" . $char_o_diaeresis_nfd, 2, "ab" ],
		vec[ "abc" . $char_o_diaeresis_nfd, 3, "abc" ],
		vec[ "abc" . $char_o_diaeresis_nfd, 4, "abc" ],
		vec[ "abc" . $char_o_diaeresis_nfd, 5, "abc" . $char_o_diaeresis_nfd],
		vec[ "abc" . $char_o_diaeresis_nfd, 6, "abc" . $char_o_diaeresis_nfd],
		vec[ $char_o_diaeresis_nfd . "abc", 0, "" ],
		vec[ $char_o_diaeresis_nfd . "abc", 1, "" ],
		vec[ $char_o_diaeresis_nfd . "abc", 2, $char_o_diaeresis_nfd ],
		vec[ $char_o_diaeresis_nfd . "abc", 3, $char_o_diaeresis_nfd . "a" ],
		vec[ $char_o_diaeresis_nfd . "abc", 4, $char_o_diaeresis_nfd . "ab" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 5, $char_o_diaeresis_nfd . "abc" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 6, $char_o_diaeresis_nfd . "abc" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 7, $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "x" ],

		vec[ "abc", 3, 0, "abc" ],
		vec[ "abc", 2, 1, "bc" ],
		vec[ "abc", 1, 2, "c" ],
		vec[ "abc", 0, 3, "false" ],
		vec[ "abc", 1, 3, "false" ],
		vec[ "abc", 1, 999, "false" ],
		vec[ $char_o_diaeresis_nfd . "abc", 1, 6, "false" ],
		vec[ $char_o_diaeresis_nfd . "abc", 1, 999, "false" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 0, $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "x" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 1, $char_diaeresis . "abc" . $char_a_ring_nfd . "xy" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 2, "abc" . $char_a_ring_nfd . "xyz" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 3, "abc" . $char_a_ring_nfd . "xyz" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 4, "bc" . $char_a_ring_nfd . "xyz" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 5, "c" . $char_a_ring_nfd . "xyz" ],
		vec[ $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 6, $char_a_ring_nfd . "xyz" ],

	];

	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "extract from \"$arg0\" \"".$test[1]."\" graphemes - grapheme_extract GRAPHEME_EXTR_MAXCHARS";
		if ( 3 == count( $test ) ) {
            $tmp = null;
            $result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXCHARS, 0, inout $tmp);
		}
		else {
			$res_str .= " starting at byte position ".$test[2];
            $tmp = null;
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXCHARS, $test[2], inout $tmp);
		}
		$res_str .= " = ";
		if ( $result === false ) {
			$res_str .= 'false';
		}
		else {
			$res_str .= urlencode($result);
		}
		$res_str .= " == " . urlencode($test[count($test)-1]) . check_result($result, $test[count($test)-1]) . "\n";
	}


	//=====================================================================================

	return $res_str;
}

function check_result($result, $expected) :mixed{

	if ( $result === false ) {
        	$result = 'false';
	}

	if ( strcmp((string)$result, (string)$expected) != 0 ) {
		return " **FAILED** ";
	}

	return "";
}
<<__EntryPoint>>
function main_entry(): void {

  echo ut_main();
}
