<?php

// posotive steps
var_dump(range(2147483400, 2147483600, 100));
var_dump( range(2147483646, 2147483648, 1 ) );
var_dump( range(2147483646, 2147483657, 1 ) );
var_dump( range(2147483630, 2147483646, 5 ) );
 
// negative steps  
var_dump( range(-2147483645, -2147483648, 1 ) );
var_dump( range(-2147483645, -2147483649, 1 ) );
var_dump( range(-2147483630, -2147483646, 5 ) );

// low > high
var_dump(range(2147483647, 2147483645, 1 ));
var_dump(range(2147483648, 2147483645, 1 ));

?>