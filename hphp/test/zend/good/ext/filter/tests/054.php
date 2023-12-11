<?hh
<<__EntryPoint>> function main(): void {
$data = dict['foo' => 123];

var_dump(
	filter_var_array($data, dict['foo' => dict['filter' => FILTER_DEFAULT], 'bar' => dict['filter' => FILTER_DEFAULT]], false),
	filter_var_array($data, dict['foo' => dict['filter' => FILTER_DEFAULT], 'bar' => dict['filter' => FILTER_DEFAULT]])
);
}
