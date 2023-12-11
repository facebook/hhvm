<?hh

/*
 * Format a number using misc locales/patterns.
 */

function ut_main()
:mixed{

$pattern=<<<_MSG_
{0, select,
  female {{1, plural, offset:1
      =0 {{2} does not give a party.}
      =1 {{2} invites {3} to her party.}
      =2 {{2} invites {3} and one other person to her party.}
     other {{2} invites {3} as one of the # people invited to her party.}}}
  male   {{1, plural, offset:1
      =0 {{2} does not give a party.}
      =1 {{2} invites {3} to his party.}
      =2 {{2} invites {3} and one other person to his party.}
     other {{2} invites {3} as one of the # other people invited to his party.}}}
  other {{1, plural, offset:1
      =0 {{2} does not give a party.}
      =1 {{2} invites {3} to their party.}
      =2 {{2} invites {3} and one other person to their party.}
      other {{2} invites {3} as one of the # other people invited to their party.}}}}
_MSG_;


$args = vec[
      dict[0 => 'female', 1 => 0,  2 => 'Alice', 3 => 'Bob'],
      dict[0 => 'male',   1 => 1,  2 => 'Alice', 3 => 'Bob'],
      dict[0 => 'none',   1 => 2,  2 => 'Alice', 3 => 'Bob'],
      dict[0 => 'female', 1 => 27, 2 => 'Alice', 3 => 'Bob'],
];

$str_res = '';

        $fmt = ut_msgfmt_create( 'en_US', $pattern );
		if(!$fmt) {
			$str_res .= dump(intl_get_error_message())."\n";
			return $str_res;
		}
        foreach ($args as $arg) {
            $str_res .= dump( ut_msgfmt_format($fmt, $arg) ). "\n";
            $str_res .= dump( ut_msgfmt_format_message('en_US', $pattern, $arg) ) . "\n";
    }
    return $str_res;
}

<<__EntryPoint>>
function main_entry(): void {
    include_once( 'ut_common.inc' );
    // Run the test
    ut_run();
}
