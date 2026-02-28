<?hh

/*
 * Format a number using misc locales/patterns.
 */

function ut_main()
:mixed{
    $locales = dict[
        'en_US' => "{0,number,integer} monkeys on {1,number,integer} trees make {2,number} monkeys per tree",
        'ru_UA' => "{0,number,integer} \xd0\xbc\xd0\xb0\xd0\xb2\xd0\xbf \xd0\xbd\xd0\xb0 {1,number,integer} \xd0\xb4\xd0\xb5\xd1\x80\xd0\xb5\xd0\xb2\xd0\xb0\xd1\x85 \xd1\x86\xd0\xb5 {2,number} \xd0\xbc\xd0\xb0\xd0\xb2\xd0\xbf\xd0\xb8 \xd0\xbd\xd0\xb0 \xd0\xba\xd0\xbe\xd0\xb6\xd0\xbd\xd0\xbe\xd0\xbc\xd1\x83 \xd0\xb4\xd0\xb5\xd1\x80\xd0\xb5\xd0\xb2i",
        'de' => "{0,number,integer} Affen \xc3\xbcber {1,number,integer} B\xc3\xa4ume um {2,number} Affen pro Baum",
        'en_UK' => "{0,number,integer} monkeys on {1,number,integer} trees make {2,number} monkeys per tree",
	'root' => '{0,whatever} would not work!',
	'fr' => 'C\'est {0,number,integer}',
    ];

	$results = dict[
		'en_US' => "4,560 monkeys on 123 trees make 37.073 monkeys per tree",
		'ru_UA' => "4\xc2\xa0560 \xd0\xbc\xd0\xb0\xd0\xb2\xd0\xbf \xd0\xbd\xd0\xb0 123 \xd0\xb4\xd0\xb5\xd1\x80\xd0\xb5\xd0\xb2\xd0\xb0\xd1\x85 \xd1\x86\xd0\xb5 37,073 \xd0\xbc\xd0\xb0\xd0\xb2\xd0\xbf\xd0\xb8 \xd0\xbd\xd0\xb0 \xd0\xba\xd0\xbe\xd0\xb6\xd0\xbd\xd0\xbe\xd0\xbc\xd1\x83 \xd0\xb4\xd0\xb5\xd1\x80\xd0\xb5\xd0\xb2i",
		'de' => "4.560 Affen \xc3\xbcber 123 B\xc3\xa4ume um 37,073 Affen pro Baum",
		'en_UK' => "4,560 monkeys on 123 trees make 37.073 monkeys per tree",
		'root' => "4,560 monkeys on 123 trees make 37.073 monkeys per tree",
		'fr' => "C'est 42",

	];

	$str_res = '';

    foreach( $locales as $locale => $pattern )
    {
        $str_res .= "\nLocale is: $locale\n";
        $fmt = ut_msgfmt_create( $locale, $pattern );
		if(!$fmt) {
			$str_res .= dump(intl_get_error_message())."\n";
			continue;
		}
        $str_res .= dump( ut_msgfmt_parse( $fmt, $results[$locale] ) ) . "\n";
		$str_res .= dump( ut_msgfmt_parse_message($locale, $pattern, $results[$locale])) . "\n";
    }
    return $str_res;
}

<<__EntryPoint>>
function main_entry(): void {
	include_once( 'ut_common.inc' );
  // Run the test
  ut_run();
}
