<?hh


<<__EntryPoint>>
function main_preg_cache_full() :mixed{
$matches = 1;
for ($i=1 ; $i < 100000 ; $i++) {
  $db_name = 'dbs.'.rand();
  $match = null;
  if (preg_match_with_matches("/^dbs\.(\d+)$/", $db_name, inout $match)) {
    $db_num = $match[1];
    $printable_db_name = preg_replace('/' .$db_num.'/', '%d', $db_name);
    if (!$printable_db_name) {
      var_dump("preg_replace returned false");
      break;
    }
    ++$matches;
  }
}
var_dump(sprintf("%d matches", $matches));
}
