<?hh <<__EntryPoint>> function main(): void {
$pdo = new PDO('sqlite:' . __SystemLib\hphp_test_tmppath('foo.db'));

$attrs = varray[PDO::ATTR_STATEMENT_CLASS, PDO::ATTR_STRINGIFY_FETCHES, PDO::NULL_TO_STRING];

foreach ($attrs as $attr) {
    var_dump($pdo->setAttribute($attr, NULL));
    var_dump($pdo->setAttribute($attr, 1));
    var_dump($pdo->setAttribute($attr, 'nonsense'));
}

unlink(__SystemLib\hphp_test_tmppath('foo.db'));
}
