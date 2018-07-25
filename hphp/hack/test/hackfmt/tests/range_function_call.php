<?hh

f(
  self::getBuilder()
    ->setSomeProperty(self::$thing)
    ->setSomeOtherProperty(self::$ids['stuff'])
    ->genSave(),
  self::getBuilder()
    ->setAnotherProperty(self::$item)
    ->setYetAnotherProperty(self::$value)
    ->setOneMoreProperty(vec[self::$junk])
    ->genSave(),
);
