<?hh


<<__EntryPoint>>
function main_ext_options() :mixed{
var_dump(dl(""));

var_dump(extension_loaded("bcmath"));
var_dump(extension_loaded("curl"));
var_dump(extension_loaded("simplexml"));
var_dump(extension_loaded("mysql"));
var_dump(extension_loaded("date"));
var_dump(extension_loaded("datetime"));
$x = get_loaded_extensions();
var_dump(!($x ?? false));

var_dump(get_included_files()[0] === __FILE__);
var_dump(vec[]);

$sec = null;
$nsec = null;
clock_getres(CLOCK_THREAD_CPUTIME_ID, inout $sec, inout $nsec);
var_dump($sec);
var_dump($nsec);
$sec = null;
$nsec = null;
clock_gettime(CLOCK_THREAD_CPUTIME_ID, inout $sec, inout $nsec);
var_dump($sec is nonnull);
var_dump($nsec is nonnull);

var_dump(ini_get(""));
var_dump(ini_get("setting_that_does_not_exist"));
ini_set("memory_limit", 50000000);
var_dump(ini_get("memory_limit"));
set_time_limit(30);
var_dump(ini_get("max_execution_time"));
ini_set("max_execution_time", 40);
var_dump(ini_get("max_execution_time"));

var_dump(phpversion());
var_dump(phpversion('mysql'));
var_dump(phpversion('memcached'));
var_dump(phpversion('exif'));
var_dump(phpversion('reflection'));
var_dump(phpversion('nonexistent_extension'));

var_dump(putenv("FOO=bar"));
var_dump(!putenv("FOO"));

var_dump(!version_compare("1.3.0.dev", "1.1.2", "<"));

$arr = get_defined_constants(true);
var_dump(count($arr["user"]) === 2);
var_dump(HH\Lib\C\any($arr["Core"], $elt ==> HH\Lib\Legacy_FIXME\eq($elt, 'PHP_OS')));
}
