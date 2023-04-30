<?hh


<<__EntryPoint>>
function main_curl_open_basedir() {
var_dump(ini_set('open_basedir', sys_get_temp_dir()));

$ch = curl_init();
var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, ""));
var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, sys_get_temp_dir()."/foo"));
var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, "/xxx/bar"));
}
