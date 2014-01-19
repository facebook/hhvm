<?php

$dyadic = 0.00000000000045474735088646411895751953125;
var_dump(printf ("%1.41f\n",unserialize(serialize($dyadic))));

?>