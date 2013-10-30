<?php
$failures = array(
    true,
    false,
    0,
    1,
    "string",
    new stdclass,
    array(),
    array('$id'),
    array('$ref'),
);

class myclass {
    public function __construct() {
        $this->{'$id'} = 1;
        $this->{'$ref'} = 2;
    }
}

$success    = array(
    MongoDBRef::create("collection", "someid"),
    array('$ref' => 'reference', '$id' => 'theids'),
    (object)array('$ref' => 'reference', '$id' => 'theids'),
    new myclass,
);


echo "These should fail\n";
foreach($failures as $val) {
    var_dump(MongoDBRef::isRef($val));
}

echo "These should pass\n";
foreach($success as $val) {
    var_dump(MongoDBRef::isRef($val));
}
?>
===DONE===
<?php exit(0); ?>
