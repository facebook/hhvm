<?hh


<<__EntryPoint>>
function main_include_path_parse_ini() :mixed{
$direct_ini_file = "include_path_parse_direct_dir.ini";
$nested_ini_file = "include_path_parse_nested_dir.ini";
$nested_dir = "ini_files";

$full_direct_ini_file = __DIR__.'/'.$direct_ini_file;
$full_nested_ini_file = __DIR__.'/'.$nested_dir.'/'.$nested_ini_file;

$initial_cwd = getcwd();

// Empty, fail.
var_dump(parse_ini_file(''));

// Direct: relative (from containing file path).
var_dump(parse_ini_file($direct_ini_file));

// Nested: relative (containing file path, fail).
var_dump(parse_ini_file($nested_ini_file));

// Direct: relative from CWD path.
chdir(__DIR__);
var_dump(parse_ini_file($direct_ini_file));

// Absolute paths.
chdir($initial_cwd);
var_dump(parse_ini_file($full_direct_ini_file));
var_dump(parse_ini_file($full_nested_ini_file));

// Nested: relative from include path.
set_include_path(__DIR__.'/'.$nested_dir);
var_dump(parse_ini_file($nested_ini_file));

// For compressed files, search on the include path isn't supported
$compress = "compress.zlib://";
$compress_suffix = ".gz";
$direct_compress_ini_file = $compress.$direct_ini_file.$compress_suffix;
$nested_compress_ini_file = $compress.$nested_dir.'/'.$nested_ini_file
    .$compress_suffix;
$full_direct_compress_ini_file = $compress.__DIR__.'/'.$direct_ini_file
    .$compress_suffix;
$fail_compress_ini_file = $compress.__DIR__.'/'.$nested_ini_file
    .$compress_suffix;

// Relative to source (php) file
var_dump(parse_ini_file($direct_compress_ini_file));

// Direct with compression
chdir(__DIR__);
var_dump(parse_ini_file($direct_compress_ini_file));

// Relative path with dirs and compression
var_dump(parse_ini_file($nested_compress_ini_file));

// Full path.
chdir($initial_cwd);
var_dump(parse_ini_file($full_direct_compress_ini_file));

// And a failure to ensure
var_dump(parse_ini_file($fail_compress_ini_file));

// Validate that http-based files fail. Note that the URL isn't valid,
// but really we're just testing what happens if "http://" is the prefix.
$http_file = "http://".$full_direct_ini_file;
var_dump(parse_ini_file($http_file));
}
