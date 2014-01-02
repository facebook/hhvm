<?php

try {
    $mongo = new MongoClient('mongodb://127.0.0.1/', array(
        'connect' => true,
        'connectTimeoutMS' => 500,
    ));
    $db = $mongo->selectDB('hhvm_test');
    $table = 'test';

    $db->$table->insert(array('key' => 'val', 'key2' => 'val2'), array('safe' => true));
    // in hhvm this throws a duplicate, in php it works
    $db->$table->insert(array('key' => 'val', 'key2' => 'val2'), array('safe' => true));

    // in hhvm the result is empty
    $cursor = $db->$table->find();
    var_dump($cursor->count() > 0);

} catch (Exception $e) {
    var_dump($e->getMessage());
}

