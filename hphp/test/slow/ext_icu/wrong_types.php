<?hh

class MyFakeCalendar0 {}
class MyFakeCalendar1 {}
class MyFakeCalendar2 {}
class MyFakeCalendar3 {}

<<__EntryPoint>>
function main() :mixed{
  $calendar = IntlCalendar::createInstance('UTC');
  var_dump($calendar->after(new MyFakeCalendar0()));
  var_dump($calendar->before(new MyFakeCalendar1()));
  var_dump($calendar->equals(new MyFakeCalendar2()));
  var_dump($calendar->isEquivalentTo(new MyFakeCalendar3()));
}
