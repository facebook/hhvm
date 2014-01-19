<?hh
function f() {
  var_dump(Vector {
}
);
  var_dump(Map {
}
);
  var_dump(Vector {
1, 2}
);
  var_dump(StableMap {
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
  var_dump(StableMap<string,int> {
'a' => 1, 'b' => 2}
);
}
f();
