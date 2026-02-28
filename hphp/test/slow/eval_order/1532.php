<?hh

trait T1532 {
}

<<__EntryPoint>>
function main_1532() :mixed{
  var_dump(class_uses("A1532", false));
  var_dump(class_uses("A1532"));
  var_dump(class_exists("A1532"));
}
