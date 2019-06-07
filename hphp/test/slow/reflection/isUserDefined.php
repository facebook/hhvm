<?hh


<<__EntryPoint>>
function main_is_user_defined() {
var_dump((new ReflectionClass('ReflectionClass'))->isUserDefined());
}
