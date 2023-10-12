<?hh

$p1 = new Point;     // error2038
$p1 = new Point();  // legal
$p1 = new Point 12, 34;  // error2038, among others
$p1 = new Point(12);   // legal
$p1 = new RReeaallllyyLLoonnggNNaammee;   // error2038
$p1 = new $PointClassVar->$pointClassName();  // legal
$p1 = new 'Point'(12);  // not a class-type-designator
$p1 = new Point::Point(12);  // not a class-type-designator
$p1 = new Point::$pointVar(12);  // legal
$p1 = new Point::Point::$pointVar(12);  // not a class-type-designator
$p1 = new Point::$pointVar::$pointVar(12);  // legal
$p1 = new $point(12); // legal
$p1 = new Point<int>(12); // legal
$p1 = new (function_that_returns_class_name())(12); // legal
$p1 = "Point" |> new $$(12); // legal
