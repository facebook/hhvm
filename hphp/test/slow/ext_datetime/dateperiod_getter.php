<?hh


<<__EntryPoint>>
function main_dateperiod_getter() :mixed{
$period = new DatePeriod(
  new DateTime("2015-01-01"),
  new DateInterval("PT6H"),
  new DateTime("2015-01-02")
);


$period->getStartDate()->modify("+7 days"); // Make sure we got clone right
$period->getEndDate()->modify("+7 days");

echo 'Starting Date = ', $period->getStartDate()->format('Y-m-d'), PHP_EOL;
echo 'Ending Date = ', $period->getEndDate()->format('Y-m-d'), PHP_EOL;
echo 'Interval = ', $period->getDateInterval()->format('%H hours'), PHP_EOL;
}
