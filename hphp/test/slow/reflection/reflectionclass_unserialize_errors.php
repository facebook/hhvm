<?hh
<<__EntryPoint>> function main(): void {
var_dump(unserialize("O:15:\"ReflectionClass\":3:{s:4:\"name\";s:8:\"Collator\";s:20:\"\000ReflectionClass\000obj\";N;s:7:\"\0native\";i:1337;}"));
var_dump(unserialize("O:15:\"ReflectionClass\":3:{s:4:\"name\";s:1:\"A\";s:20:\"\000ReflectionClass\000obj\";N;s:7:\"\0native\";s:1:\"A\";}"));
}
