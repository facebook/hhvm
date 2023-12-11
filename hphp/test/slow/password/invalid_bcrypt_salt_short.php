<?hh <<__EntryPoint>> function main(): void {
password_hash('foo', PASSWORD_BCRYPT, dict["salt" => 'abc']);
}
