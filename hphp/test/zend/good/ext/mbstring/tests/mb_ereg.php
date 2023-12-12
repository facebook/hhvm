<?hh

	function test_ereg( $test_enc, $pat, $str, $in_enc = 'EUC-JP' ) :mixed{
		mb_regex_encoding( $test_enc );
		$pat = mb_convert_encoding( $pat, $test_enc, $in_enc );
		$str = mb_convert_encoding( $str, $test_enc, $in_enc );
		$reg = null;
		printf( "(%d)%s\n", mb_ereg( $pat, $str, inout $reg ), ( is_array( $reg )? bin2hex(mb_convert_encoding( implode( b' ', $reg ), $in_enc, $test_enc )) : '' ) );
	}
	function do_tests( $enc ) :mixed{
		test_ereg( $enc, b'abc ([a-z]+) ([a-z]+) ([a-z]+)$', b"abc def ghi jkl" );
		$pat = b"([\xa3\xe1-\xa3\xfa]+) ([ \xa4\xa2-\xa4\xab]+)([\xa4\xab-\xa4\xca]+) ([\xa4\xef-\xa4\xf3]+)$";
		test_ereg( $enc, $pat, b"\xa3\xe1\xa3\xe2\xa3\xe3 \xa4\xa2\xa4\xaa\xa4\xa4 \xa4\xab\xa4\xb3\xa4\xca \xa4\xef\xa4\xf1\xa4\xf3" );
		test_ereg( $enc, $pat, b"\xa3\xed\xa3\xfa\xa3\xf8\xa3\xe6\xa3\xf0 \xa4\xa6\xa4\xaa\xa4\xab \xa4\xad\xa4\xab \xa4\xf2\xa4\xf0" );
	}
<<__EntryPoint>>
function main_entry(): void {
  	mb_regex_set_options( '' );

  	$encs = vec[ 'EUC-JP', 'Shift_JIS', 'SJIS', 'UTF-8' ];

  	foreach( $encs as $enc ) {
  		do_tests( $enc );
  	}
}
