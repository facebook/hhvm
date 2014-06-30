<?php

// Ensuring that inconsistent constructor use results in an error to avoid
// problems creeping in.

trait TNew {
    public function __construct() {
        echo "TNew executed\n";
    }
}

class ReportCollision {
    use TNew;
	
	public function ReportCollision() {
	    echo "ReportCollision executed\n";
	}
}


echo "ReportCollision: ";
$o = new ReportCollision;

