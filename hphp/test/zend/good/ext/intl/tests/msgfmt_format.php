<?hh

/*
 * Format a number using misc locales/patterns.
 */

function ut_main()
:mixed{
    $locales = dict[
        'en_US' => "{0,number,integer} monkeys on {1,number,integer} trees make {2,number} monkeys per tree",
        'ru_UA' => "{0,number,integer} мавп на {1,number,integer} деревах це {2,number} мавпи на кожному деревi",
        'de' => "{0,number,integer} Affen über {1,number,integer} Bäume um {2,number} Affen pro Baum",
        'en_UK' => "{0,number,integer} monkeys on {1,number,integer} trees make {2,number} monkeys per tree",
	'root' => '{0,whatever} would not work!',
	'fr' => "C'est la vie!",
    ];

    $str_res = '';
	$m = 4560;
	$t = 123;

    foreach( $locales as $locale => $pattern )
    {
        $str_res .= "\nLocale is: $locale\n";
        $fmt = ut_msgfmt_create( $locale, $pattern );
		if(!$fmt) {
			$str_res .= dump(intl_get_error_message())."\n";
			continue;
		}
        $str_res .= dump( ut_msgfmt_format( $fmt, dict[0 => $m, 1 => $t, 2 => $m/$t] ) ) . "\n";
		$str_res .= dump( ut_msgfmt_format_message($locale, $pattern, dict[0 => $m, 1 => $t, 2 => $m/$t])) . "\n";
    }
    return $str_res;
}

<<__EntryPoint>>
function main_entry(): void {
  include_once( 'ut_common.inc' );
  // Run the test
  ut_run();
}
