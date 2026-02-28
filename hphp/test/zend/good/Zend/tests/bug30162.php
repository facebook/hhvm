<?hh
class FIIFO {

    public function __construct() {
        $this->x = "x";
        throw new Exception;
    }

}

class hariCow extends FIIFO {

    public function __construct() {
        try {
            parent::__construct();
        } catch(Exception $e) {
        }
        $this->y = "y";
        try {
            $this->z = new FIIFO;
        } catch(Exception $e) {
        }
    }

    public function __toString() :mixed{
        return "Rusticus in asino sedet.";
    }

}
<<__EntryPoint>> function main(): void {
try {
    $db = new FIIFO();
    var_dump($db);
} catch(Exception $e) {
}

$db = new hariCow;

var_dump($db);
echo "===DONE===\n";
}
