<?hh

class :foo {}

<<__EntryPoint>>
function main_empty_xhp() {
error_reporting(-1);

var_dump(empty(<foo/>));
}
