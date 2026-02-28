<?hh

class MyFakeCalendar0 {}
class MyFakeCalendar1 {}
class MyFakeCalendar2 {}
class MyFakeCalendar3 {}

<<__EntryPoint>>
function main(): mixed {
  set_error_handler(($_, $msg) ==> {
    throw new Exception($msg);
  });

  $calendar = IntlCalendar::createInstance('UTC');
  try { $calendar->after(new MyFakeCalendar0()); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $calendar->before(new MyFakeCalendar1()); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $calendar->equals(new MyFakeCalendar2()); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { $calendar->isEquivalentTo(new MyFakeCalendar3()); } catch (Exception $e) { var_dump($e->getMessage()); }
}
