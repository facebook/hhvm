<?hh

<<__EntryPoint>>
function main_interval_dates() :mixed{
$interval = new \DateInterval('2007-03-01T13:00:00Z/2008-05-11T15:30:00Z');
var_dump($interval->y);
var_dump($interval->m);
var_dump($interval->d);
var_dump($interval->h);
var_dump($interval->i);
var_dump($interval->s);
}
