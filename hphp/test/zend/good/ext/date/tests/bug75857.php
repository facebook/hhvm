<?hh

<<__EntryPoint>> function main(): void {
$longDate = new DateTime('now', new DateTimeZone('America/Argentina/ComodRivadavia'));
$mediumDate = new DateTime('now', new DateTimeZone('America/Indiana/Indianapolis'));
$smallDate = new DateTime('now', new DateTimeZone('America/Sao_Paulo'));

var_dump($longDate->format('e'));
var_dump($mediumDate->format('e'));
var_dump($smallDate->format('e'));
}
