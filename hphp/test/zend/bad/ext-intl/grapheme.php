<?php

/*
 * Test grapheme functions (procedural only)
 */

function ut_main()
{
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

	$tests = array(
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2 ),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 1 ),
		array( "abc", $char_a_ring_nfd, "false" ),
		array( $char_a_ring_nfd . "bc", "a", "false" ),
		array( "abc", "d", "false" ),
		array( "abc", "c", 2 ),
		array( "abc", "b", 1 ),
		array( "abc", "a", 0 ),
		array( "abc", "a", 0, 0 ),
		array( "abc", "a", 1, "false" ),
		array( "ababc", "a", 1, 2 ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ),
		
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "op", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "opq", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 2 ),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 1 ),
		array( "abc", $char_a_ring_nfd . "bc", "false" ),
		array( $char_a_ring_nfd . "bc", "abcdefg", "false" ),
		array( "abc", "defghijklmnopq", "false" ),
		array( "abc", "ab", 0 ),
		array( "abc", "bc", 1 ),
		array( "abc", "abc", 0 ),
		array( "abc", "abcd", "false" ),
		array( "abc", "ab", 0, 0 ),
		array( "abc", "abc", 0, 0 ),
		array( "abc", "abc", 1, "false" ),
		array( "ababc", "ab", 1, 2 ),
		array( "ababc", "abc", 1, 2 ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_a_ring_nfd . "bc", "o" . $char_a_ring_nfd . "bc", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "bc" . $char_a_ring_nfd, 2, 3 ),
	);

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strpos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strpos($test[0], $test[1]);
		}
		else {
			$res_str .= " from $test[2]";
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
	
	$tests = array(
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "O", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_O_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd, 2 ),
		array( "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 1 ),
		array( "Abc", $char_a_ring_nfd, "false" ),
		array( $char_a_ring_nfd . "bc", "A", "false" ),
		array( "abc", "D", "false" ),
		array( "abC", "c", 2 ),
		array( "abc", "B", 1 ),
		array( "Abc", "a", 0 ),
		array( "abc", "A", 0, 0 ),
		array( "Abc", "a", 1, "false" ),
		array( "ababc", "A", 1, 2 ),
		
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "oP", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "opQ", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bC" . $char_o_diaeresis_nfd, $char_O_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "Bc", $char_A_ring_nfd . "bc", 2 ),
		array( "a" . $char_a_ring_nfd . "BC", $char_a_ring_nfd . "bc", 1 ),
		array( "abc", $char_a_ring_nfd . "BC", "false" ),
		array( $char_a_ring_nfd . "BC", "aBCdefg", "false" ),
		array( "aBC", "Defghijklmnopq", "false" ),
		array( "abC", "Ab", 0 ),
		array( "aBC", "bc", 1 ),
		array( "abC", "Abc", 0 ),
		array( "abC", "aBcd", "false" ),
		array( "ABc", "ab", 0, 0 ),
		array( "aBc", "abC", 0, 0 ),
		array( "abc", "aBc", 1, "false" ),
		array( "ABabc", "AB", 1, 2 ),
		array( "abaBc", "aBc", 1, 2 ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_A_ring_nfd . "bC", "O" . $char_a_ring_nfd . "bC", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bC" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "Bc" . $char_a_ring_nfd, 2, 3 ),
	);

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_stripos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_stripos($test[0], $test[1]);
		}
		else {
			$res_str .= " from $test[2]";
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


	$tests = array(
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2 ),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 1 ),
		array( "abc", $char_a_ring_nfd, "false" ),
		array( $char_a_ring_nfd . "bc", "a", "false" ),
		array( "abc", "d", "false" ),
		array( "abc", "c", 2 ),
		array( "abc", "b", 1 ),
		array( "abc", "a", 0 ),
		array( "abc", "a", 0, 0 ),
		array( "abc", "a", 1, "false" ),
		array( "ababc", "a", 1, 2 ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ),
		
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "op", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "opq", "opq", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 2 ),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", 1 ),
		array( "abc", $char_a_ring_nfd . "bc", "false" ),
		array( $char_a_ring_nfd . "bc", "abcdefg", "false" ),
		array( "abc", "defghijklmnopq", "false" ),
		array( "abc", "ab", 0 ),
		array( "abc", "bc", 1 ),
		array( "abc", "abc", 0 ),
		array( "abc", "abcd", "false" ),
		array( "abc", "ab", 0, 0 ),
		array( "abc", "abc", 0, 0 ),
		array( "abc", "abc", 1, "false" ),
		array( "ababc", "ab", 1, 2 ),
		array( "ababc", "abc", 1, 2 ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_a_ring_nfd . "bc", "o" . $char_a_ring_nfd . "bc", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_a_ring_nfd . "bc" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "bc" . $char_a_ring_nfd, 2, 3 ),
	);

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strrpos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strrpos($test[0], $test[1]);
		}
		else {
			$res_str .= " from $test[2]";
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
	
	$tests = array(
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 2, 3 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "O", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_O_diaeresis_nfd, $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd, 2 ),
		array( "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, 1 ),
		array( "Abc", $char_a_ring_nfd, "false" ),
		array( $char_a_ring_nfd . "bc", "A", "false" ),
		array( "abc", "D", "false" ),
		array( "abC", "c", 2 ),
		array( "abc", "B", 1 ),
		array( "Abc", "a", 0 ),
		array( "abc", "A", 0, 0 ),
		array( "Abc", "a", 1, "false" ),
		array( "ababc", "A", 1, 2 ),
		
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "oP", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", "opQ", 5 ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "abc", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "bC" . $char_o_diaeresis_nfd, $char_O_diaeresis_nfd . "bc" . $char_o_diaeresis_nfd, 4 ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "Bc", $char_A_ring_nfd . "bc", 2 ),
		array( "a" . $char_a_ring_nfd . "BC", $char_a_ring_nfd . "bc", 1 ),
		array( "abc", $char_a_ring_nfd . "BC", "false" ),
		array( $char_a_ring_nfd . "BC", "aBCdefg", "false" ),
		array( "aBC", "Defghijklmnopq", "false" ),
		array( "abC", "Ab", 0 ),
		array( "aBC", "bc", 1 ),
		array( "abC", "Abc", 0 ),
		array( "abC", "aBcd", "false" ),
		array( "ABc", "ab", 0, 0 ),
		array( "aBc", "abC", 0, 0 ),
		array( "abc", "aBc", 1, "false" ),
		array( "ABabc", "AB", 1, 2 ),
		array( "abaBc", "aBc", 1, 2 ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o" . $char_A_ring_nfd . "bC", "O" . $char_a_ring_nfd . "bC", 2, 6 ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bC" . $char_a_ring_nfd . "def", $char_a_ring_nfd . "Bc" . $char_a_ring_nfd, 2, 3 ),
	);

	foreach( $tests as $test ) {
	    $arg1 = urlencode($test[1]);
	    $arg0 = urlencode($test[0]);
		$res_str .= "find \"$arg1\" in \"$arg0\" - grapheme_strripos";
		if ( 3 == count( $test ) ) {
			$result = grapheme_strripos($test[0], $test[1]);
		}
		else {
			$res_str .= " from $test[2]";
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

	$tests = array(

		array( "abc", 3, "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, 5, "false" ),
		array( "ao" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", 2, $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ),
		array( $char_o_diaeresis_nfd . $char_a_ring_nfd . "a" . $char_A_ring_nfd . "bc", 2, "a" . $char_A_ring_nfd . "bc" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", 5, "O" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, 5, "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_O_diaeresis_nfd, 4, $char_O_diaeresis_nfd ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", 2, $char_a_ring_nfd . "bc" ),
		array( "a" . $char_A_ring_nfd . "bc", 1, $char_A_ring_nfd . "bc" ),
		array( "Abc", -5, "false" ),
		array( $char_a_ring_nfd . "bc", 3, "false" ),
		array( "abc", 4, "false" ),
		array( "abC", 2, "C" ),
		array( "abc", 1, "bc" ),
		array( "Abc", 1, 1, "b" ),
		array( "abc", 0, 2, "ab" ),
		array( "Abc", -4, 1, "false" ),
		array( "ababc", 1, 2, "ba" ),
		array( "ababc", 0, 10, "ababc" ),
		
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, 10 , "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -1, "Op" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -2, "O" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -3, "" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 5, -4, "false" ),

		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -1, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Op" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -2, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -3, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -4, "a" . $char_a_ring_nfd . "bc" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -5, "a" . $char_a_ring_nfd . "b" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -6, "a" . $char_a_ring_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -7, "a" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -8, "" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", 0, -9, "false" ),

		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -7, $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -6, "bc" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -5, "c" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -4, $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -3, "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -2, "pq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -1, "q" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -999, "false" ),

		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 8, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 7, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Op" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 6, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 5, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 4, "a" . $char_a_ring_nfd . "bc" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 3, "a" . $char_a_ring_nfd . "b" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 2, "a" . $char_a_ring_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 1, "a" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, 0, "" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -999, "false" ),

		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -1, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Op" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -2, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -3, "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -4, "a" . $char_a_ring_nfd . "bc" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -5, "a" . $char_a_ring_nfd . "b" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -6, "a" . $char_a_ring_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -7, "a" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -8, "" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "Opq", -8, -9, "false" ),
		
	);

	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "substring of \"$arg0\" from \"$test[1]\" - grapheme_substr";
		if ( 3 == count( $test ) ) {
			$result = grapheme_substr($test[0], $test[1]);
		}
		else {
			$res_str .= " with length $test[2]";
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

	$tests = array(
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "o", "o", "o" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_o_diaeresis_nfd, $char_o_diaeresis_nfd ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, $char_a_ring_nfd . "bc"),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, $char_a_ring_nfd . "bc"),
		array( "abc", $char_a_ring_nfd, "false" ),
		array( $char_a_ring_nfd . "bc", "a", "false" ),
		array( "abc", "d", "false" ),
		array( "abc", "c", "c" ),
		array( "abc", "b", "bc" ),
		array( "abc", "a", "abc" ),
		array( "abc", "ab", "abc" ),
		array( "abc", "abc", "abc" ),
		array( "abc", "bc", "bc" ),
		array( "abc", "a", FALSE, "abc" ),
		array( "abc", "a", TRUE, "" ),
		array( "abc", "b", TRUE, "a" ),
		array( "abc", "c", TRUE, "ab" ),
		array( "ababc", "bab", TRUE, "a" ),
		array( "ababc", "abc", TRUE, "ab" ),
		array( "ababc", "abc", FALSE, "abc" ),
		
		array( "ab" . $char_a_ring_nfd . "c", "d", "false" ),
		array( "bc" . $char_a_ring_nfd . "a", "a", "a" ),
		array( "a" . $char_a_ring_nfd . "bc", "b", "bc" ),
		array( $char_a_ring_nfd . "bc", "a", "false" ),
		array( $char_a_ring_nfd . "abc", "ab", "abc" ),
		array( "abc" . $char_a_ring_nfd, "abc", "abc" . $char_a_ring_nfd),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc", $char_a_ring_nfd . "bc" ),
		array( "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, FALSE, $char_a_ring_nfd . "bc"),
		array( "a" . $char_a_ring_nfd . "bc", "a", TRUE, "" ),
		array( $char_a_ring_nfd . "abc", "b", TRUE, $char_a_ring_nfd . "a" ),
		array( "ab" . $char_a_ring_nfd . "c", "c", TRUE, "ab" . $char_a_ring_nfd ),
		array( "aba" . $char_a_ring_nfd . "bc", "ba" . $char_a_ring_nfd . "b", TRUE, "a" ),
		array( "ababc" . $char_a_ring_nfd, "abc" . $char_a_ring_nfd, TRUE, "ab" ),
		array( "abab" . $char_a_ring_nfd . "c", "ab" . $char_a_ring_nfd . "c", FALSE, "ab" . $char_a_ring_nfd . "c" ),

	);

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

	$tests = array(
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, $char_O_diaeresis_nfd, $char_o_diaeresis_nfd ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd . "O", "o", "O" ),
		array( "a" . $char_a_ring_nfd . "bc" . $char_o_diaeresis_nfd, "o", "false" ),
		array( $char_o_diaeresis_nfd . "a" . $char_a_ring_nfd . "bc", $char_a_ring_nfd, $char_a_ring_nfd . "bc"),
		array( "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd, $char_a_ring_nfd . "bc"),
		array( "abc", $char_a_ring_nfd, "false" ),
		array( $char_a_ring_nfd . "bc", "A", "false" ),
		array( "abc", "d", "false" ),
		array( "abc", "C", "c" ),
		array( "aBc", "b", "Bc" ),
		array( "abc", "A", "abc" ),
		array( "abC", "ab", "abC" ),
		array( "abc", "aBc", "abc" ),
		array( "abC", "bc", "bC" ),
		array( "abc", "A", FALSE, "abc" ),
		array( "abc", "a", TRUE, "" ),
		array( "aBc", "b", TRUE, "a" ),
		array( "abc", "C", TRUE, "ab" ),
		array( "aBabc", "bab", TRUE, "a" ),
		array( "ababc", "aBc", TRUE, "ab" ),
		array( "ababc", "abC", FALSE, "abc" ),
		
		array( "ab" . $char_a_ring_nfd . "c", "d", "false" ),
		array( "bc" . $char_a_ring_nfd . "A", "a", "A" ),
		array( "a" . $char_a_ring_nfd . "bc", "B", "bc" ),
		array( $char_A_ring_nfd . "bc", "a", "false" ),
		array( $char_a_ring_nfd . "abc", "Ab", "abc" ),
		array( "abc" . $char_A_ring_nfd, "abc", "abc" . $char_A_ring_nfd),
		array( "a" . $char_a_ring_nfd . "bc", $char_A_ring_nfd . "bc", $char_a_ring_nfd . "bc" ),
		array( "a" . $char_A_ring_nfd . "bc", $char_a_ring_nfd, FALSE, $char_A_ring_nfd . "bc" ),
		array( "a" . $char_a_ring_nfd . "bc", "A", TRUE, "" ),
		array( $char_a_ring_nfd . "aBc", "b", TRUE, $char_a_ring_nfd . "a" ),
		array( "ab" . $char_a_ring_nfd . "c", "C", TRUE, "ab" . $char_a_ring_nfd ),
		array( "aba" . $char_A_ring_nfd . "bc", "ba" . $char_a_ring_nfd . "b", TRUE, "a" ),
		array( "ababc" . $char_a_ring_nfd, "aBc" . $char_A_ring_nfd, TRUE, "ab" ),
		array( "abAB" . $char_A_ring_nfd . "c", "ab" . $char_a_ring_nfd . "c", FALSE, "AB" . $char_A_ring_nfd . "c" ),

	);

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

	$tests = array(
		// haystack, count, [[offset], [next]], result
		array( "abc", 3, "abc" ),
		array( "abc", 2, "ab" ),
		array( "abc", 1, "a" ),
		array( "abc", 0, "" ),
		array( "abc", 1, 0, "a" ),
		array( "abc", 1, 1, "b" ),
		array( "abc", 1, 2, "c" ),
		array( "abc", 0, 2, "" ),

		array( "abc", 3, 0, 3, "abc" ),
		array( "abc", 2, 0, 2, "ab" ),
		array( "abc", 1, 0, 1, "a" ),
		array( "abc", 0, 0, 0, "" ),
		array( "abc", 1, 0, 1, "a" ),
		array( "abc", 1, 1, 2, "b" ),
		array( "abc", 1, 2, 3, "c" ),
		array( "abc", 0, 2, 2, "" ),
        array( "http://news.bbc.co.uk/2/hi/middle_east/7831588.stm", 48, 48 , 50 , "tm" ),

		array( $char_a_ring_nfd . "bc", 3, $char_a_ring_nfd . "bc" ),
		array( $char_a_ring_nfd . "bc", 2, $char_a_ring_nfd . "b" ),
		array( $char_a_ring_nfd . "bc", 1, $char_a_ring_nfd . "" ),
		array( $char_a_ring_nfd . "bc", 3, 0, 5, $char_a_ring_nfd . "bc" ),
		array( $char_a_ring_nfd . "bc", 2, 0, 4, $char_a_ring_nfd . "b" ),
		array( $char_a_ring_nfd . "bc", 1, 0, 3, $char_a_ring_nfd . "" ),
		array( $char_a_ring_nfd . "bcde", 2, 3, 5, "bc" ),
		array( $char_a_ring_nfd . "bcde", 2, 4, 6, "cd" ),
		array( $char_a_ring_nfd . "bcde" . $char_a_ring_nfd . "f", 4, 5, 11, "de" . $char_a_ring_nfd . "f" ),

		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, $char_a_ring_nfd . $char_o_diaeresis_nfd ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 1, $char_a_ring_nfd . "" ),

		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 0, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 2, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 3, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 1, 4, $char_diaeresis),

		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 0, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 2, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 3, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 4, $char_diaeresis . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 7, $char_diaeresis . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 8, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 10, $char_diaeresis),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 11, "false"),

	);

	$next = -1;
	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "extract from \"$arg0\" \"$test[1]\" graphemes - grapheme_extract";
		if ( 3 == count( $test ) ) {
			$result = grapheme_extract($test[0], $test[1]);
		}
		elseif ( 4 == count ( $test ) ) {
			$res_str .= " starting at byte position $test[2]";
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_COUNT, $test[2]);
		}
		else {
			$res_str .= " starting at byte position $test[2] with \$next";
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_COUNT, $test[2], $next);
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
			$res_str .= " \$next=$next == $test[3] ";
			if ( $next != $test[3] ) {
				$res_str .= "***FAILED***";
			}
		}
		$res_str .= "\n";
	}
	

	//=====================================================================================
	$res_str .= "\n" . 'function grapheme_extract($haystack, $size, $extract_type = GRAPHEME_EXTR_MAXBYTES, $start = 0)' . "\n\n";

	$tests = array(
		array( "abc", 3, "abc" ),
		array( "abc", 2, "ab" ),
		array( "abc", 1, "a" ),
		array( "abc", 0, "" ),
		array( $char_a_ring_nfd . "bc", 5, $char_a_ring_nfd . "bc" ),
		array( $char_a_ring_nfd . "bc", 4, $char_a_ring_nfd . "b" ),
		array( $char_a_ring_nfd . "bc", 1, "" ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 9, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 10, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 11, $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, $char_a_ring_nfd . $char_o_diaeresis_nfd ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 3, $char_a_ring_nfd . "" ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 4, $char_a_ring_nfd . "" ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 5, $char_a_ring_nfd . "" ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 6, $char_a_ring_nfd . $char_o_diaeresis_nfd  ),
		array( $char_a_ring_nfd . $char_o_diaeresis_nfd . "c", 7, $char_a_ring_nfd . $char_o_diaeresis_nfd . "c" ),

		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 0, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 2, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 3, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 4, $char_diaeresis),

		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, 0, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, 2, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 6, 3, $char_o_diaeresis_nfd . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 5, 4, $char_diaeresis . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 5, 7, $char_diaeresis . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 3, 8, $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 10, $char_diaeresis),
		array( $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd . $char_o_diaeresis_nfd, 2, 11, "false"),

	);

	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "extract from \"$arg0\" \"$test[1]\" graphemes - grapheme_extract GRAPHEME_EXTR_MAXBYTES";
		if ( 3 == count( $test ) ) {
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXBYTES);
		}
		else {
			$res_str .= " starting at byte position $test[2]";
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXBYTES, $test[2]);
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

	$tests = array(
		array( "abc", 3, "abc" ),
		array( "abc", 2, "ab" ),
		array( "abc", 1, "a" ),
		array( "abc", 0, "" ),
		array( "abc" . $char_o_diaeresis_nfd, 0, "" ),
		array( "abc" . $char_o_diaeresis_nfd, 1, "a" ),
		array( "abc" . $char_o_diaeresis_nfd, 2, "ab" ),
		array( "abc" . $char_o_diaeresis_nfd, 3, "abc" ),
		array( "abc" . $char_o_diaeresis_nfd, 4, "abc" ),
		array( "abc" . $char_o_diaeresis_nfd, 5, "abc" . $char_o_diaeresis_nfd),
		array( "abc" . $char_o_diaeresis_nfd, 6, "abc" . $char_o_diaeresis_nfd),
		array( $char_o_diaeresis_nfd . "abc", 0, "" ),
		array( $char_o_diaeresis_nfd . "abc", 1, "" ),
		array( $char_o_diaeresis_nfd . "abc", 2, $char_o_diaeresis_nfd ),
		array( $char_o_diaeresis_nfd . "abc", 3, $char_o_diaeresis_nfd . "a" ),
		array( $char_o_diaeresis_nfd . "abc", 4, $char_o_diaeresis_nfd . "ab" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 5, $char_o_diaeresis_nfd . "abc" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 6, $char_o_diaeresis_nfd . "abc" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 7, $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "x" ),

		array( "abc", 3, 0, "abc" ),
		array( "abc", 2, 1, "bc" ),
		array( "abc", 1, 2, "c" ),
		array( "abc", 0, 3, "false" ),
		array( "abc", 1, 3, "false" ),
		array( "abc", 1, 999, "false" ),
		array( $char_o_diaeresis_nfd . "abc", 1, 6, "false" ),
		array( $char_o_diaeresis_nfd . "abc", 1, 999, "false" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 0, $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "x" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 1, $char_diaeresis . "abc" . $char_a_ring_nfd . "xy" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 2, "abc" . $char_a_ring_nfd . "xyz" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 3, "abc" . $char_a_ring_nfd . "xyz" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 4, "bc" . $char_a_ring_nfd . "xyz" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 5, "c" . $char_a_ring_nfd . "xyz" ),
		array( $char_o_diaeresis_nfd . "abc" . $char_a_ring_nfd . "xyz", 8, 6, $char_a_ring_nfd . "xyz" ),

	);

	foreach( $tests as $test ) {
	    $arg0 = urlencode($test[0]);
		$res_str .= "extract from \"$arg0\" \"$test[1]\" graphemes - grapheme_extract GRAPHEME_EXTR_MAXCHARS";
		if ( 3 == count( $test ) ) {
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXCHARS);
		}
		else {
			$res_str .= " starting at byte position $test[2]";
			$result = grapheme_extract($test[0], $test[1], GRAPHEME_EXTR_MAXCHARS, $test[2]);
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

echo ut_main();

function check_result($result, $expected) {

	if ( $result === false ) {
        	$result = 'false';
	}

	if ( strcmp($result, $expected) != 0 ) {
		return " **FAILED** ";
	}

	return "";
}

?>