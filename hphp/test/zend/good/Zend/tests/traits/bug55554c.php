<?hh

// Test that the behavior is consistent with the existing handling of new
// and legacy constructors.
// Here, the traits conflicts are overridden by local definitions,
// and the two constructor definitions do not directly collide in that case.

trait TC1 {
    public function __construct() {
        echo "TC1 executed\n";
    }
    public function ReportCollision() :mixed{
        echo "TC1 executed\n";
    }
}

trait TC2 {
    public function __construct() {
        echo "TC2 executed\n";
    }
    public function ReportCollision() :mixed{
        echo "TC1 executed\n";
    }
}

class ReportCollision {
    use TC1, TC2;

    public function __construct() {
        echo "New constructor executed\n";
    }
    public function ReportCollision() :mixed{
        echo "Legacy constructor executed\n";
    }
}

<<__EntryPoint>> function main(): void {
echo "ReportCollision: ";
$o = new ReportCollision;
}
