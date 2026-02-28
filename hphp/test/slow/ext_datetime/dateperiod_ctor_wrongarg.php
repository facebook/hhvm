<?hh


<<__EntryPoint>>
function main_dateperiod_ctor_wrongarg() :mixed{
try {
  new DatePeriod(new DateTime("now"), new DateInterval("P2Y4DT6H8M"), "wrong");
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
