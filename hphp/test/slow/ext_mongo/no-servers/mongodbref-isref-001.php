<?hh <<__EntryPoint>> function main(): void {
var_dump(MongoDBRef::isRef(darray['$ref' => 'dbref', '$id' => 123]));
var_dump(MongoDBRef::isRef(darray['$ref' => 'dbref', '$id' => new MongoId()]));
var_dump(MongoDBRef::isRef(darray['$ref' => 'dbref', '$id' => 123, '$db' => 'test']));
var_dump(MongoDBRef::isRef((object) darray['$ref' => 'dbref', '$id' => 123]));
var_dump(MongoDBRef::isRef((object) darray['$ref' => 'dbref', '$id' => new MongoId()]));
var_dump(MongoDBRef::isRef((object) darray['$ref' => 'dbref', '$id' => 123, '$db' => 'test']));
var_dump(MongoDBRef::isRef(null));
var_dump(MongoDBRef::isRef(1));
var_dump(MongoDBRef::isRef(varray[]));
var_dump(MongoDBRef::isRef(darray['$ref' => 'dbref']));
var_dump(MongoDBRef::isRef(darray['$id' => 123, '$db' => 'test']));
var_dump(MongoDBRef::isRef((object) varray[]));
var_dump(MongoDBRef::isRef((object) darray['$ref' => 'dbref']));
var_dump(MongoDBRef::isRef((object) darray['$id' => 123, '$db' => 'test']));
}
