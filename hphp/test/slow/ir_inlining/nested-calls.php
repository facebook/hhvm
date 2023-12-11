<?hh

function get_region($s) :mixed{
  echo "get_region\n";
  return $s ? 1 : 0;
}
function get_local_cluster_id() :mixed{
  return c::thing();
}
function get_local_region() :mixed{
  return get_region(get_local_cluster_id());
}
class c {
  public function __construct() {
    $local1 = new stdClass;
    $local2 = vec[];
    get_local_region();
  }

  public static function thing() :mixed{
    return rand() ? 1 : 0;
  }
}


<<__EntryPoint>>
function main_nested_calls() :mixed{
new c;
echo "done\n";
}
