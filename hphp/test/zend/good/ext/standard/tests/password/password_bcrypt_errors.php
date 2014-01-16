<?php
//-=-=-=-

var_dump(password_hash("foo", PASSWORD_BCRYPT, array("cost" => 3)));

var_dump(password_hash("foo", PASSWORD_BCRYPT, array("cost" => 32)));

var_dump(password_hash("foo", PASSWORD_BCRYPT, array("salt" => "foo")));

var_dump(password_hash("foo", PASSWORD_BCRYPT, array("salt" => "123456789012345678901")));

var_dump(password_hash("foo", PASSWORD_BCRYPT, array("salt" => 123)));

var_dump(password_hash("foo", PASSWORD_BCRYPT, array("cost" => "foo")));

?>