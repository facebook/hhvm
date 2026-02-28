<?hh

function main() :mixed{
  var_dump(vec[vec[]] === vec[vec[]]);
}
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(($_, $errmsg) ==> {
    throw new Exception($errmsg);
  });

  try {
    main();
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
