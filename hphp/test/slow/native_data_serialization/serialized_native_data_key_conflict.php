<?hh

// When a private property is serialized, the key is \0class\0name.
// Since the key of serialized native data \0native, we want to make
// sure that it does not conflict with any properties in any possible
// way.
class native {
  private $native = 1337;
}


<<__EntryPoint>>
function main_serialized_native_data_key_conflict() :mixed{
$serialized = serialize(new native());
var_dump(json_encode($serialized));
var_dump(unserialize($serialized));
}
