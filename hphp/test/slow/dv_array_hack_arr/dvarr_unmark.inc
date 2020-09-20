<?hh

function foo(int $x) {
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	varray[1, 2, 3]
    	)
  	)
	));  
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	varray[$x, $x + 1]
    	)
  	)
	));  
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	darray['a' => 1]
    	)
  	)
	));  
  var_dump(HH\is_array_marked_legacy(
  	HH\array_unmark_legacy(
    	HH\array_mark_legacy(
      	darray['a' => $x]
    	)
  	)
	));  
}
