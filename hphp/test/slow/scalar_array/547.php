<?hh
function func($params) {
 var_dump($params);
}


const VALUE = 1;
<<__EntryPoint>>
function main_547() {
func(darray['key' => @VALUE]);
}
