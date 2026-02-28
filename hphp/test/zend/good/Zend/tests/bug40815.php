<?hh

class ehandle{
  <<__DynamicallyCallable>> static public function exh ($ex) :mixed{
    echo 'foo';
  }
}
<<__EntryPoint>> function main(): void {
set_exception_handler("ehandle::exh");

throw new Exception ("Whiii");
echo "Done\n";
}
