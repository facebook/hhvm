<?hh

function ut_main($bundle) :mixed{
	$str_res = '';
	// fall back
	$r = ut_resourcebundle_create( 'en_US', $bundle );

	$str_res .= sprintf( "length: %d\n", ut_resourcebundle_count($r) );
	$str_res .= sprintf( "teststring: %s\n", ut_resourcebundle_get($r,  'teststring' ) );
	$str_res .= sprintf( "testint: %d\n", ut_resourcebundle_get($r, 'testint' ) );

	$str_res .= print_r( ut_resourcebundle_get($r, 'testvector' ), true );

	$str_res .= sprintf( "testbin: %s\n", bin2hex(ut_resourcebundle_get( $r,'testbin' )) );

	$r2 = ut_resourcebundle_get($r, 'testtable' );
	$str_res .= sprintf( "testtable: %d\n", ut_resourcebundle_get($r2, 'major' ) );

	$r2 = ut_resourcebundle_get($r,'testarray' );
	$str_res .= sprintf( "testarray: %s\n", ut_resourcebundle_get($r2, 2 ) );

	$t = ut_resourcebundle_get( $r, 'nonexisting' );
	$str_res .= debug( $t );

	return $str_res;
}
function ut_run2($bundle) :mixed{
			// Run unit test in OO mode.
			\HH\global_set('oo-mode', true);
			$oo_result = ut_main($bundle);

			// Run unit test in procedural mode.
			\HH\global_set('oo-mode', false);
			$proc_result = ut_main($bundle);

			// Show error if the APIs produce different results.
			if( $proc_result !== $oo_result )
			{
						echo "ERROR: OO- and procedural APIs produce different results!\n";
						echo "OO API output:\n";
						echo str_repeat( '=', 78 ) . "\n";
						echo $oo_result;
						echo str_repeat( '=', 78 ) . "\n";
						echo "procedural API output:\n";
						echo str_repeat( '=', 78 ) . "\n";
						echo $proc_result;
						echo str_repeat( '=', 78 ) . "\n";
						return;
			}

			// Else, if the results are equal, show one of them.
			echo $proc_result;
}
<<__EntryPoint>>
function main_entry(): void {
  include "resourcebundle.inc";
  include_once( 'ut_common.inc' );

  ut_run2(bundle());
}
