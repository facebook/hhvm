<?hh // strict

$p1 = new Point;     // error2038
$p1 = new Point();  // legal
$p1 = new Point 12, 34;  // error2038, among others
$p1 = new Point(12);   // legal
$p1 = new RReeaallllyyLLoonnggNNaammee;   // error2038
$p1 = NEW Point(12);   // legal, but maybe should not be.
