<?hh

class X {
 static function g() :mixed{
}
}

<<__EntryPoint>>
function main_167() :mixed{
;
echo 'abc' . (string)(X::g()) . 'efg';
}
