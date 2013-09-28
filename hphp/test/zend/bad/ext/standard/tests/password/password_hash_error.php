<?php
//-=-=-=-

var_dump(password_hash());

var_dump(password_hash("foo"));

var_dump(password_hash("foo", array()));

var_dump(password_hash("foo", 19, new StdClass));

var_dump(password_hash("foo", PASSWORD_BCRYPT, "baz"));

var_dump(password_hash(array(), PASSWORD_BCRYPT));

var_dump(password_hash("123", PASSWORD_BCRYPT, array("salt" => array())));

/* Non-string salt, checking for memory leaks */
var_dump(password_hash('123', PASSWORD_BCRYPT, array('salt' => 1234)));

?>