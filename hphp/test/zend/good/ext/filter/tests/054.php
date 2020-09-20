<?hh
<<__EntryPoint>> function main(): void {
$data = darray['foo' => 123];

var_dump(
	filter_var_array($data, darray['foo' => darray['filter' => FILTER_DEFAULT], 'bar' => darray['filter' => FILTER_DEFAULT]], false),
	filter_var_array($data, darray['foo' => darray['filter' => FILTER_DEFAULT], 'bar' => darray['filter' => FILTER_DEFAULT]])
);
}
