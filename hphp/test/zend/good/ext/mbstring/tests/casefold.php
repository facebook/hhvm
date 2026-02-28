<?hh
<<__EntryPoint>>
function main_entry(): void {
  	mb_internal_encoding( 'ISO-8859-1' );
  	print mb_strtolower( "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" );
  	print mb_strtoupper( mb_strtolower( "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n" ) );
  	print mb_strtoupper( "\xe4\xeb\xef\n" );
  	print mb_convert_case( "\xe4\xeb\xef\n", MB_CASE_TITLE );
}
