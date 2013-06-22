<?php
//-=-=-=-
var_dump(password_needs_rehash());

var_dump(password_needs_rehash(''));

var_dump(password_needs_rehash('', "foo"));

var_dump(password_needs_rehash(array(), 1));

var_dump(password_needs_rehash("", 1, "foo"));

echo "OK!";
?>