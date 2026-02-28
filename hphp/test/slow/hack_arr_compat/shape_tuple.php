<?hh

<<__DynamicallyCallable>>
function handle_error($errno, $errstr, ...$args) :mixed{
  print("(Notice: $errstr) ");
  return true;
}

function check<reify T>($x): T {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);

  print("\n===================================\nTesting tuple:\n");
  print("\nUsing empty tuple:\n"); $tuple = vec[];
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
  print("\nUsing empty shape:\n"); $shape = dict[];
  print('varray: '); var_dump(varray($shape) is shape(...));
  print('darray: '); var_dump(darray($shape) is shape(...));
  print('vec:    '); var_dump(vec($shape)    is shape(...));
  print('dict:   '); var_dump(dict($shape)   is shape(...));
  print('keyset: '); var_dump(keyset($shape) is shape(...));
}
