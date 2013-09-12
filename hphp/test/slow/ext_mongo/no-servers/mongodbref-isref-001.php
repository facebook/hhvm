<?php
var_dump(MongoDBRef::isRef(array('$ref' => 'dbref', '$id' => 123)));
var_dump(MongoDBRef::isRef(array('$ref' => 'dbref', '$id' => new MongoId())));
var_dump(MongoDBRef::isRef(array('$ref' => 'dbref', '$id' => 123, '$db' => 'test')));
var_dump(MongoDBRef::isRef((object) array('$ref' => 'dbref', '$id' => 123)));
var_dump(MongoDBRef::isRef((object) array('$ref' => 'dbref', '$id' => new MongoId())));
var_dump(MongoDBRef::isRef((object) array('$ref' => 'dbref', '$id' => 123, '$db' => 'test')));
var_dump(MongoDBRef::isRef(null));
var_dump(MongoDBRef::isRef(1));
var_dump(MongoDBRef::isRef(array()));
var_dump(MongoDBRef::isRef(array('$ref' => 'dbref')));
var_dump(MongoDBRef::isRef(array('$id' => 123, '$db' => 'test')));
var_dump(MongoDBRef::isRef((object) array()));
var_dump(MongoDBRef::isRef((object) array('$ref' => 'dbref')));
var_dump(MongoDBRef::isRef((object) array('$id' => 123, '$db' => 'test')));
?>
