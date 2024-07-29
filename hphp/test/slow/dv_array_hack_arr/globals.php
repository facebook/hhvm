<?hh


<<__EntryPoint>>
function main() :mixed{
  $dicts = vec[
    tuple("cookie", \HH\global_get('_COOKIE')),
    tuple("env", \HH\global_get('_ENV')),
    tuple("files", \HH\global_get('_FILES')),
    tuple("get", \HH\global_get('_GET')),
    tuple("post", \HH\global_get('_POST')),
    tuple("request", \HH\global_get('_REQUEST')),
    tuple("server", \HH\global_get('_SERVER')),
  ];
  $dfuncs = vec[
    HH\is_php_array<>,
    HH\is_darray<>,
    HH\is_dict<>,
  ];
  foreach ($dicts as list($name, $d)) {
    echo "$name:\n";
    foreach ($dfuncs as $f) {
      echo "\t".HH\fun_get_function($f).": ".var_export($f($d), true)."\n";
    }
  }
}
