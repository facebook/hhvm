<?php
/* From http://bugs.php.net/19865 */
echo var_export(explode("\1", "a". chr(1). "b". chr(0). "d" . chr(1) . "f" . chr(1). "1" . chr(1) . "d"), TRUE);
echo md5(var_export(explode("\1", "a". chr(1). "b". chr(0). "d" . chr(1) . "f" . chr(1). "1" . chr(1) . "d"), TRUE));
echo "\n";
var_dump(@explode("", ""));
var_dump(@explode("", NULL));
var_dump(@explode(NULL, ""));
var_dump(@explode("a", ""));
var_dump(@explode("a", "a"));
var_dump(@explode("a", NULL));
var_dump(@explode(NULL, a));
var_dump(@explode("abc", "acb"));
var_dump(@explode("somestring", "otherstring"));
var_dump(@explode("somestring", "otherstring", -1));
var_dump(@explode("a", "aaaaaa"));
var_dump(@explode("==", str_repeat("-=".ord(0)."=-", 10)));
var_dump(@explode("=", str_repeat("-=".ord(0)."=-", 10)));
//////////////////////////////////////
var_dump(explode(":","a lazy dog:jumps:over:",-1));
var_dump(explode(":","a lazy dog:jumps:over", -1));
var_dump(explode(":","a lazy dog:jumps:over", -2));
var_dump(explode(":","a lazy dog:jumps:over:",-4));
var_dump(explode(":","a lazy dog:jumps:over:",-40000000000000));
var_dump(explode(":^:","a lazy dog:^:jumps::over:^:",-1));
var_dump(explode(":^:","a lazy dog:^:jumps::over:^:",-2));
?>