<?hh


<<__EntryPoint>>
function main_hash() {
echo strlen(password_hash('foo', PASSWORD_BCRYPT))."\n";

$hash = password_hash('foo', PASSWORD_BCRYPT);
echo ($hash == crypt('foo', $hash) ? "yes" : "no")."\n";
echo "\n";

echo password_hash("rasmusledorf", PASSWORD_BCRYPT,
  darray["cost" => 7, "salt" => "usesomesillystringforsalt"])."\n";
echo password_hash("test", PASSWORD_BCRYPT,
  darray["salt" => "123456789012345678901" . chr(0)])."\n";

echo password_hash('', PASSWORD_BCRYPT,
                   darray["salt" => "1234567890123456789012345678901234567890"]);
}
