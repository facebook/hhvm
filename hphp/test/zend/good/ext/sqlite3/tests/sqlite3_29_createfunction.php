<?hh


<<__EntryPoint>>
function main_entry(): void {

  $db = new SQLite3(':memory:');

  $func = 'strtoupper';
  var_dump($db->createfunction($func, $func));
  var_dump($db->querysingle('SELECT strtoupper("test")'));

  $func2 = 'strtolower';
  var_dump($db->createfunction($func2, $func2));
  var_dump($db->querysingle('SELECT strtolower("TEST")'));

  var_dump($db->createfunction($func, $func2));
  var_dump($db->querysingle('SELECT strtoupper("tEst")'));
}
