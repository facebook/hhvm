<?hh

class X{}
<<__EntryPoint>>
function main_arg_checking() :mixed{
;

$utc = new \DateTimeZone('UTC');
$amsterdam = new \DateTimeZone('Europe/Amsterdam');
$date = new \DateTime("2012-10-10");

var_dump(DateTime::createFromFormat('j-M-Y', '15-Feb-2009', new X()));
var_dump($date->setTimezone(new X()));
var_dump($amsterdam->getOffset($utc));
var_dump($amsterdam->getOffset($date));
var_dump($date->add($utc));
var_dump($date->sub($utc));
var_dump($date->diff(new X()));
var_dump(date_format(new X(), "Y"));
var_dump(date_create("2012-10-10", new X()));
}
