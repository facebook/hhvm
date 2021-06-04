<?hh

<<__EntryPoint>>
function test() : void {
  setcookie('name', 'value1');
  setcookie('name', 'value2');
  setcookie('Name', 'value3', 0, '/asdf');
  setcookie('name', 'value4', 0, '/', '.facebook.com');
  $headers = HH\Lib\Vec\sort(headers_list());
  foreach ($headers as $header) {
    echo " $header";
  }
}
