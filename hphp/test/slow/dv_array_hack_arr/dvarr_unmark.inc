<?hh

function foo(int $x) {
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	vec[1, 2, 3]
    	)
  	)
	));
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	vec[$x, $x + 1]
    	)
  	)
	));
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	dict['a' => 1]
    	)
  	)
	));
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	dict['a' => $x]
    	)
  	)
	));
}
