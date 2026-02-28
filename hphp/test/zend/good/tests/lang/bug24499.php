<?hh
class Id {
        private $id="priv";

        public function tester($obj)
:mixed        {
                $obj->id = "bar";
        }
}
<<__EntryPoint>>
function main() :mixed{
  $id = new Id();
  $obj = new stdClass();
  $obj->foo = "bar";
  $id->tester($obj);
  print_r($obj);
}
