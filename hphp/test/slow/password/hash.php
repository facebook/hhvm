<?php

echo strlen(password_hash('foo', PASSWORD_BCRYPT))."\n";

$hash = password_hash('foo', PASSWORD_BCRYPT);
echo ($hash == crypt('foo', $hash) ? "yes" : "no")."\n";
echo "\n";

echo password_hash("rasmusledorf", PASSWORD_BCRYPT,
  ["cost" => 7, "salt" => "usesomesillystringforsalt"])."\n";
echo password_hash("test", PASSWORD_BCRYPT,
  ["salt" => "123456789012345678901" . chr(0)])."\n";
