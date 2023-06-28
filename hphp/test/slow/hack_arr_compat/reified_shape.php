<?hh

function check<reify T>($x): T {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  print('darray: '); print(json_encode(check<shape('a' => int)>(darray[]))."\n");
  print('varray: '); print(json_encode(check<shape('a' => int)>(varray[]))."\n");
}
