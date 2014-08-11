<?php

var_dump(gmp_init("98765678"));
var_dump(gmp_strval(gmp_init("98765678")));
var_dump(gmp_strval(gmp_init()));
var_dump(gmp_init());
var_dump(gmp_init(1,2,3,4));
var_dump(gmp_init(1,-1));
var_dump(gmp_init("",36));
var_dump(gmp_init("foo",3));
var_dump(gmp_strval(gmp_init("993247326237679187178",3)));

echo "Done\n";
?>
