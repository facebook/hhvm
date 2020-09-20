<?hh <<__EntryPoint>> function main(): void {
var_dump(Reflection::getModifierNames(ReflectionMethod::IS_FINAL | ReflectionMethod::IS_PROTECTED));
}
