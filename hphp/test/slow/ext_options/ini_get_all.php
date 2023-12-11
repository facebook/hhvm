<?hh


<<__EntryPoint>>
function main_ini_get_all() :mixed{
$all_detailed = ini_get_all();
var_dump($all_detailed['hphp.compiler_version']['access']);
var_dump($all_detailed['allow_url_fopen']);

$all_short = ini_get_all('', false);
var_dump($all_short['allow_url_fopen']);

$pcre = ini_get_all('pcre');
ksort(inout $pcre);
var_dump($pcre);
$pcre_false = ini_get_all('pcre', false);
ksort(inout $pcre_false);
var_dump($pcre_false);

$core = ini_get_all('core');
var_dump(dict[
  'core: allow_url_fopen' => isset($core['allow_url_fopen']),
  'core: pcre.backtrack_limit' => isset($core['pcre.backtrack_limit']),
]);

ini_get_all("THIS_EXTENSION_SHOULD_NOT_EXIST");
}
