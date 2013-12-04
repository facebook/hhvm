<?php
ini_set('precision', 12);

// this checks f,g,G conversion for snprintf/spprintf
var_dump(array(ini_get('precision'),.012,-.012,.12,-.12,1.2,-1.2,12.,-12.,0.000123,.0000123,123456789012.0,1234567890123.0,12345678901234567890.0));
?>