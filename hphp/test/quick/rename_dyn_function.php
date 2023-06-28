<?hh

function a() :mixed{
  echo "Hello from a\n";
}

function b() :mixed{
  echo "I am b\n";
}

function callfns($name, $name2) :mixed{
  echo "Calling $name\n";
  $name();
  echo "Calling $name2\n";
  $name2();
}
<<__EntryPoint>> function main(): void {
callfns('a', 'b');
fb_rename_function('a', 'old_a');
fb_rename_function('b', 'a');
fb_rename_function('old_a', 'b');
callfns('a', 'b');
}
