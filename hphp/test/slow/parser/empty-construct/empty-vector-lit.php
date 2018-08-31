<?hh


<<__EntryPoint>>
function main_empty_vector_lit() {
error_reporting(-1);

var_dump(empty(Vector {1, 2, 3}));
}
