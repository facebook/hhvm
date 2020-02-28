<?hh

function a() {
  $foo = varray["bob", "fred", "jussi", "jouni", "egon", "marliese"];
  $bar = each(inout $foo);
  var_dump($bar);
}

function b() {
  $foo = darray["Robert" => "Bob", "Seppo" => "Sepi"];
  $bar = each(inout $foo);
  var_dump($bar);
}

function c() {
  $fruit = darray["a" => "apple", "b" => "banana", "c" => "cranberry"];
  reset(inout $fruit);
  $output = '';
  while (true) {
    $item = each(inout $fruit);
    if ($item === false) break;
    $output .= $item[0];
    $output .= ": ";
    $output .= $item[1];
    $output .= "\n";
  }
  var_dump($output);
}


<<__EntryPoint>>
function main_each() {
a();
b();
c();
}
