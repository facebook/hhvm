<?php
class foo {}
<<__EntryPoint>> function main() {
$result = get_declared_classes();
var_dump(array_search('foo', $result));
}
