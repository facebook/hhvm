<?hh

<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $message, ...$_) ==> {
    echo $message, "\n";
    return true;
  });
  mb_ereg_search_init('hello');
  var_dump(mb_ereg_search());
  var_dump(mb_ereg_search_init('hello', 'hello'));
  var_dump(mb_ereg_search());
}
