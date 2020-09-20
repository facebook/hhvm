<?hh
//-=-=-=-
<<__EntryPoint>> function main(): void {
var_dump(password_hash("foo", PASSWORD_BCRYPT, darray["cost" => 3]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, darray["cost" => 32]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, darray["salt" => "foo"]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, darray["salt" => "123456789012345678901"]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, darray["salt" => 123]));

var_dump(password_hash("foo", PASSWORD_BCRYPT, darray["cost" => "foo"]));
}
