<?hh
//-=-=-=-
<<__EntryPoint>> function main(): void {
var_dump(password_hash("foo", PASSWORD_BCRYPT, dict["cost" => 3]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, dict["cost" => 32]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, dict["salt" => "foo"]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, dict["salt" => "123456789012345678901"]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, dict["salt" => 123]));
}
