<?hh
class FIIFO {

        public function __construct() {
                throw new Exception;
        }

}

class hariCow extends FIIFO {

        public function __construct() {
                try {
                        parent::__construct();
                } catch(Exception $e) {
                }
        }

        public function __toString() :mixed{
                return "ok\n";
        }

}

<<__EntryPoint>> function main(): void {
$db = new hariCow;

echo $db;
}
