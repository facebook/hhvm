<?hh
function func($params) :mixed{
 var_dump($params);
}


const VALUE = 1;
<<__EntryPoint>>
function main_547() :mixed{
func(dict['key' => @VALUE]);
}
