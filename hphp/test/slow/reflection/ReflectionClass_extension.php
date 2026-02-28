<?hh


<<__EntryPoint>>
function main_reflection_class_extension() :mixed{
$ref = new ReflectionClass('ReflectionClass');
var_dump($ref->getExtension() is ReflectionExtension);
var_dump(is_string($ref->getExtensionName()));
}
