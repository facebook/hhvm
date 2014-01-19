<?php

// Ensuring that inconsistent constructor use results in an error to avoid
// problems creeping in.

trait TLegacy {
    public function ReportCollision() {
        echo "TLegacy executed\n";
    }
}

class ReportCollision {
    use TLegacy;
	
	public function __construct() {
	    echo "ReportCollision executed\n";
	}
}


echo "ReportCollision: ";
$o = new ReportCollision;

