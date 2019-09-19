<?hh
class _d extends DateTime {}
class _t extends DateTimeZone {}
<<__EntryPoint>>
function main_entry(): void {
  date_default_timezone_set('Europe/Berlin');
  $d = new _d("1pm Aug 1 GMT 2007");
  var_dump($d->format(DateTime::RFC822));
  $c = clone $d;
  var_dump($c->format(DateTime::RFC822));
  $d->modify("1 hour");
  $c->modify("1 second ago");
  var_dump($d->format(DateTime::RFC822));
  var_dump($c->format(DateTime::RFC822));
  $t = new _t("Asia/Tokyo");
  var_dump($t->getName());
  $c = clone $t;
  var_dump($c->getName());
}
