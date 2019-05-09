<?hh
<<__EntryPoint>> function main(): void {
var_dump(clone new Exception("Class Exeption should not clonable"));
}
