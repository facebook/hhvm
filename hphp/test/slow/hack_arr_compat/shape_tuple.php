<?hh

<<__DynamicallyCallable>>
function handle_error($errno, $errstr, ...$args) {
  print("(Notice: $errstr) ");
  return true;
}

function check<reify T>($x): T {
  return $x;
}

<<__EntryPoint>>
function main() {
  set_error_handler('handle_error');

  print("\n===================================\nTesting reified tuple:\n");
  print('varray: '); print(json_encode(check<(int, int)>(varray[]))."\n");
  print('darray: '); print(json_encode(check<(int, int)>(darray[]))."\n");

  print("\n===================================\nTesting reified shape:\n");
  print('varray: '); print(json_encode(check<shape('a' => int)>(varray[]))."\n");
  print('darray: '); print(json_encode(check<shape('a' => int)>(darray[]))."\n");

  print("\n===================================\nTesting tuple:\n");
  print("\nUsing empty tuple:\n"); $tuple = varray[];
  print('varray: '); var_dump(varray($tuple) is (int, int));
  print('darray: '); var_dump(darray($tuple) is (int, int));
  print('vec:    '); var_dump(vec($tuple)    is (int, int));
  print('dict:   '); var_dump(dict($tuple)   is (int, int));
  print('keyset: '); var_dump(keyset($tuple) is (int, int));
  print("\nUsing matching tuple:\n"); $tuple = tuple(17, 34);
  print('varray: '); var_dump(varray($tuple) is (int, int));
  print('darray: '); var_dump(darray($tuple) is (int, int));
  print('vec:    '); var_dump(vec($tuple)    is (int, int));
  print('dict:   '); var_dump(dict($tuple)   is (int, int));
  print('keyset: '); var_dump(keyset($tuple) is (int, int));

  print("\n===================================\nTesting shape:\n");
  print("\nUsing empty shape:\n"); $shape = shape();
  print('varray: '); var_dump(varray($shape) is shape('a' => int));
  print('darray: '); var_dump(darray($shape) is shape('a' => int));
  print('vec:    '); var_dump(vec($shape)    is shape('a' => int));
  print('dict:   '); var_dump(dict($shape)   is shape('a' => int));
  print('keyset: '); var_dump(keyset($shape) is shape('a' => int));
  print("\nUsing matching shape:\n"); $shape = shape('a' => 17);
  print('varray: '); var_dump(varray($shape) is shape('a' => int));
  print('darray: '); var_dump(darray($shape) is shape('a' => int));
  print('vec:    '); var_dump(vec($shape)    is shape('a' => int));
  print('dict:   '); var_dump(dict($shape)   is shape('a' => int));
  print('keyset: '); var_dump(keyset($shape) is shape('a' => int));

  print("\n===================================\nTesting open shape:\n");
  print("\nUsing empty shape:\n"); $shape = darray[];
  print('varray: '); var_dump(varray($shape) is shape(...));
  print('darray: '); var_dump(darray($shape) is shape(...));
  print('vec:    '); var_dump(vec($shape)    is shape(...));
  print('dict:   '); var_dump(dict($shape)   is shape(...));
  print('keyset: '); var_dump(keyset($shape) is shape(...));
}
