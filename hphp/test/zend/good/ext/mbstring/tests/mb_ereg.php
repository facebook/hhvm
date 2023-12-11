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
		$pat = b'([£á-£ú]+) ([ ¤¢-¤«]+)([¤«-¤Ê]+) ([¤ï-¤ó]+)$';
		test_ereg( $enc, $pat, b'£á£â£ã ¤¢¤ª¤¤ ¤«¤³¤Ê ¤ï¤ñ¤ó' );
		test_ereg( $enc, $pat, b'£í£ú£ø£æ£ð ¤¦¤ª¤« ¤­¤« ¤ò¤ð' );
	}
<<__EntryPoint>>
function main_entry(): void {
  	mb_regex_set_options( '' );

  	$encs = vec[ 'EUC-JP', 'Shift_JIS', 'SJIS', 'UTF-8' ];

  	foreach( $encs as $enc ) {
  		do_tests( $enc );
  	}
}
