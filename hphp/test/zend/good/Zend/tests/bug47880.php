<?hh
class bomb {
  <<__DynamicallyCallable>> static function go($n) {
    $backtrace = debug_backtrace(0);
    $backtrace[1]['args'][] = 'bomb';
  }
}
<<__EntryPoint>> function main(): void {
call_user_func_array(varray['bomb', 'go'], varray[0]);
echo "ok\n";
}
