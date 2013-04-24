<?php
	var_dump(getservbyport( -1, "tcp" ));
	var_dump(getservbyport( 80, "ppp" ));
	var_dump(getservbyport( null, null));
	var_dump(getservbyport( array(), array()));
	var_dump(getservbyport( array(80), array("tcp")));
	var_dump(getservbyport( array(2, 3), array("one"=>1, "two"=>2)));
	var_dump(getservbyport( 2, 2));
	var_dump(getservbyport( "80", "tcp"));
	var_dump(getservbyport( new stdClass(), new stdClass()));
	
?>