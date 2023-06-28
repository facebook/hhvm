<?hh

class UserClass {
}


<<__EntryPoint>>
function main_reflection_class_get_file_name_1() :mixed{
$class = new ReflectionClass('UserClass');
$parts = split("/", $class->getFileName());

var_dump($parts[count($parts) - 1]);
}
