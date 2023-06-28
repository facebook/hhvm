<?hh

function check<reify T>($x): T {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  print('varray: '); print(json_encode(check<(int, int)>(varray[]))."\n");
  print('darray: '); print(json_encode(check<(int, int)>(darray[]))."\n");
}
