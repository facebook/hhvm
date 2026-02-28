<?hh

function elemNoPromo() :mixed{
  $ret = " ";
  $ret[0] = 'A';
  return $ret;
}

function propNoPromo() :mixed{
  $ret = " ";
  $ret->prop = 'A';
  return $ret;
}

function propPromo() :mixed{
  $ret = "";
  try {
    $ret->prop = 'A';
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
  return $ret;
}
<<__EntryPoint>> function main(): void {
var_dump(elemNoPromo());
var_dump(elemNoPromo());
var_dump(propPromo());
var_dump(propPromo());
var_dump(propNoPromo());
var_dump(propNoPromo());
}
