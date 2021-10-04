<?hh

<<__EntryPoint>>
function main_hashbang() {
  var_dump(HH\facts_parse(__DIR__, vec['hash_comment.php'], true, true));
}
