<?hh
class foo {}
<<__EntryPoint>> function main(): void {
$result = get_declared_classes();
var_dump(array_search('foo', $result));
}
