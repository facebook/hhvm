<?hh

function gen1() {
    return varray[4, 5, 6];
}

function gen2() {
    yield from gen1();
}


<<__EntryPoint>>
function main_yield_from_iterator() {
$g = gen2();
foreach($g as $val) { var_dump($val); }
}
