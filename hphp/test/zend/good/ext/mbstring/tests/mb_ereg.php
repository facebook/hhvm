<?hh

	function test_ereg( $test_enc, $pat, $str, $in_enc = 'EUC-JP' ) {
		mb_regex_encoding( $test_enc );
		$pat = mb_convert_encoding( $pat, $test_enc, $in_enc );
		$str = mb_convert_encoding( $str, $test_enc, $in_enc );
		$reg = null;
		printf( "(%d)%s\n", mb_ereg( $pat, $str, inout $reg ), ( is_array( $reg )? bin2hex(mb_convert_encoding( implode( b' ', $reg ), $in_enc, $test_enc )) : '' ) );
	}
	function do_tests( $enc ) {
		test_ereg( $enc, b'abc ([a-z]+) ([a-z]+) ([a-z]+)$', b"abc def ghi jkl" );
		$pat = b'([��-��]+) ([ ��-��]+)([��-��]+) ([��-��]+)$';
		test_ereg( $enc, $pat, b'���� ������ ������ ����' );
		test_ereg( $enc, $pat, b'�������� ������ ���� ���' );
	}
<<__EntryPoint>>
function main_entry(): void {
  	mb_regex_set_options( '' );

  	$encs = varray[ 'EUC-JP', 'Shift_JIS', 'SJIS', 'UTF-8' ];

  	foreach( $encs as $enc ) {
  		do_tests( $enc );
  	}
}
