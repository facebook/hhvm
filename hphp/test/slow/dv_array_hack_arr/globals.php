<?hh


<<__EntryPoint>>
function main() {
  $dicts = vec[
    tuple("cookie", $_COOKIE),
    tuple("env", $_ENV),
    tuple("files", $_FILES),
    tuple("get", $_GET),
    tuple("post", $_POST),
    tuple("request", $_REQUEST),
    tuple("server", $_SERVER),
  ];
  $dfuncs = vec[
    fun('HH\\is_php_array'),
    fun('HH\\is_darray'),
    fun('HH\\is_dict'),
  ];
  foreach ($dicts as list($name, $d)) {
    echo "$name:\n";
    foreach ($dfuncs as $f) {
      echo "\t".HH\fun_get_function($f).": ".var_export($f($d), true)."\n";
    }
  }
}
