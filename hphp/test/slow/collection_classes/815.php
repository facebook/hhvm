<?hh
function f() :mixed{
  var_dump(Vector {
}
);
  var_dump(Map {
}
);
  var_dump(Vector {
1, 2}
);
  var_dump(Map {
'a' => 1, 'b' => 2}
);

  var_dump(Vector<int> {
}
);
  var_dump(Map<string,int> {
}
);
  var_dump(Vector<int> {
1, 2}
);
  var_dump(Map<string,int> {
'a' => 1, 'b' => 2}
);
}

<<__EntryPoint>>
function main_815() :mixed{
f();
}
