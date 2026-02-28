<?hh

function check<reify T>($x): T {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  print('varray: '); print(json_encode(check<(int, int)>(vec[]))."\n");
  print('darray: '); print(json_encode(check<(int, int)>(dict[]))."\n");
}
