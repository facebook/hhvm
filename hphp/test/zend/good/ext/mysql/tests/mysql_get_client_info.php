<?hh
include "connect.inc";
<<__EntryPoint>> function main(): void {
if (!is_string($info = mysql_get_client_info()) || ('' === $info))
    printf("[001] Expecting string/any_non_empty, got %s/%s\n", gettype($info), $info);

if ((version_compare(PHP_VERSION, '5.9.9', '>') == 1) && !is_unicode($info)) {
    printf("[002] Expecting Unicode!\n");
    var_inspect($info);
}

try { mysql_get_client_info("too many arguments"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

print "done!";
}
