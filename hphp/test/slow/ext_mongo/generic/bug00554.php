<?hh
require_once __DIR__."/../utils/server.inc";
<<__EntryPoint>> function main(): void {
$valid = varray[
    str_repeat("abcdef123456", 2),
    new MongoId,
];

$invalid = varray[
    str_repeat("klsdjf", 4),
    str_repeat("abcdef123456", 2). " ",
    new stdClass,
];

echo "VALID IDs\n";
foreach($valid as $id) {
    var_dump($id);
    var_dump(new MongoId($id));
}

echo "INVALID IDs:\n";
foreach($invalid as $id) {
    try {
        var_dump($id);
        var_dump(new MongoId($id));
    } catch(MongoException $e) {
        var_dump($e->getMessage(), $e->getCode());
    }
}
}
