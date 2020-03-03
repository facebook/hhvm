<?hh <<__EntryPoint>> function main(): void {
echo "ARRAY:\n";
try {
    $m = new MongoClient("localhost", darray["connect" => false, "timeou" => 4]);
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

try {
    $m = new MongoClient("localhost", darray["connect" => false, "readPreference" => "nearest", "slaveOkay" => true]);
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

try {
  $m = new MongoClient("localhost", darray["connect" => false, 0 => "bogus"]);
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

echo "STRING:\n";
try {
    $m = new MongoClient("mongodb://localhost/?readPreference");
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

try {
    $m = new MongoClient("mongodb://localhost/?=true");
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

try {
    $m = new MongoClient("mongodb://localhost/?timeou=4");
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

try {
    $m = new MongoClient("mongodb://localhost/?readPreference=nearest;slaveOkay=true");
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}

echo "OTHERS:\n";
MongoCursor::$slaveOkay = true;
try {
    $m = new MongoClient("localhost", darray["connect" => false, "readPreference" => "nearest"]);
} catch(Exception $e) {
    var_dump($e->getCode(), $e->getMessage());
}
}
