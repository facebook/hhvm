<?hh <<__EntryPoint>> function main() {
password_hash('foo', PASSWORD_BCRYPT, ["salt" => 'abc']);
}
