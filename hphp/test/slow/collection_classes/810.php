<?hh

$m = new Map();
$m['foo'] = new Map();
$m['foo'][123] = new Vector();
$m['foo'][123][] = 'bar';
var_dump($m['foo'][123][0]);
