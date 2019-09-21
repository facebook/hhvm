<?hh <<__EntryPoint>> function main(): void {
$result = null;
$ret = -1;
exec('/bin/echo -n -e "line1\x0Cmoreline1\r\nline2\x0C"', inout $result, inout $ret);
$content = implode($result,"\n");
var_dump(bin2hex($content));
}
