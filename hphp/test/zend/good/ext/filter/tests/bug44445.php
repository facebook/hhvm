<?php
var_dump(filter_var("foo@-foo.com",FILTER_VALIDATE_EMAIL));
var_dump(filter_var("foo@foo-.com",FILTER_VALIDATE_EMAIL));
?>