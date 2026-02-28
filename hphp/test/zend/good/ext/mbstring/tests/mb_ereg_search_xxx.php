<?hh

	function test_search( $test_enc, $str, $look_for, $opt, $in_enc = 'EUC-JP' ) :mixed{
		mb_regex_encoding( $test_enc );
		$str = mb_convert_encoding( $str, $test_enc, $in_enc );
		$look_for = mb_convert_encoding( $look_for, $test_enc, $in_enc );
		mb_ereg_search_init( $str, $look_for, $opt );
		while ( mb_ereg_search_pos() ) {
			$regs = mb_ereg_search_getregs();
			array_shift(inout $regs );
			printf( "(%s) (%d) %s\n", $test_enc, mb_ereg_search_getpos(), mb_convert_encoding( ( is_array( $regs ) ? implode( '-', $regs ): '' ), $in_enc, $test_enc ) );
		}
	}
	function do_tests( $enc, $opt ) :mixed{
		test_search( $enc, "\xa2\xcf\xa1\xa6 \xa1\xa6\xa2\xcf\n", " (\xa1\xa6?\xa2\xcf\xa1\xa6?)[[:space:]]", $opt );
		test_search( $enc, 'abcde abdeabcf anvfabc odu abcd ', '(ab[a-z]+)', $opt );
	}
<<__EntryPoint>>
function main_entry(): void {
  	mb_regex_set_options( '' );

  	$encs = vec[ 'EUC-JP', 'Shift_JIS', 'SJIS', 'UTF-8' ];

  	foreach( $encs as $enc ) {
  		do_tests( $enc, '' );
  		do_tests( $enc, 'x' );
  	}
}
