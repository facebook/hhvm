<?hh


<<__EntryPoint>>
function main_range_mem_limit() {
ini_set('memory_limit', '20M');
range(-PHP_INT_MAX - 1, -INF, PHP_INT_MAX - 1);
}
