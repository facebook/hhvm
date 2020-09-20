<?hh
class Id {
        private $id="priv";

        public function tester($obj)
        {
                $obj->id = "bar";
        }
}
<<__EntryPoint>>
function main() {
  $id = new Id();
  $obj = new stdClass();
  $obj->foo = "bar";
  $id->tester($obj);
  print_r($obj);
}
