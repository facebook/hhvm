<?hh

function bungle($s) :mixed{
  return 314159;
}

function prefix() :mixed{
  return "__rename_func_";
}
<<__EntryPoint>> function main(): void {
$stub_name = prefix() . 'strtoupper';

var_dump(strtoupper('tweet'));
fb_rename_function('strtoupper', $stub_name);
fb_rename_function('bungle', 'strtoupper');
var_dump(strtoupper('tweet'));

// release stringdata
unset($stub_name);

// try to allocate something in that memory
$stub_name = prefix() . 'hagfish';

$stub_name = prefix() . 'strtoupper';
fb_rename_function('strtoupper', 'bungle');
fb_rename_function($stub_name, 'strtoupper');
var_dump(strtoupper('tweet'));
}
