<?hh

class X {
 static function g() {
}
}

<<__EntryPoint>>
function main_167() {
;
echo 'abc' . X::g() . 'efg';
}
