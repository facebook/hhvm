<?hh
//-=-=-=-
<<__EntryPoint>> function main(): void {
var_dump(strlen(password_hash("foo", PASSWORD_BCRYPT)));

$hash = password_hash("foo", PASSWORD_BCRYPT);

var_dump($hash === crypt("foo", $hash));

var_dump(password_hash("rasmuslerdorf", PASSWORD_BCRYPT, dict["cost" => 7, "salt" => "usesomesillystringforsalt"]));

var_dump(password_hash("test", PASSWORD_BCRYPT, dict["salt" => "123456789012345678901" . chr(0)]));

echo "OK!";
}
