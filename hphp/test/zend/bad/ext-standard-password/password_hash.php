<?php
//-=-=-=-

var_dump(strlen(password_hash("foo", PASSWORD_BCRYPT)));

$hash = password_hash("foo", PASSWORD_BCRYPT);

var_dump($hash === crypt("foo", $hash));

var_dump(password_hash("rasmuslerdorf", PASSWORD_BCRYPT, array("cost" => 7, "salt" => "usesomesillystringforsalt")));

var_dump(password_hash("test", PASSWORD_BCRYPT, array("salt" => "123456789012345678901" . chr(0))));

echo "OK!";
?>