<?hh

<<__EntryPoint>>
function main_http_build_query_numeric_prefix() :mixed{
$query = dict[
    0                    => "4.6.1",
    'php'                => "7.0.99-hhvm",
    'locale'             => "de_DE",
    'mysql'              => "5.5.5",
    'local_package'      => "de_DE",
    'blogs'              => 1,
    'users'              => 5,
    8                    => 0,
    'initial_db_version' => 27916,
];
var_dump(http_build_query($query, null, "&"));
var_dump(http_build_query($query, '52', "&"));
}
