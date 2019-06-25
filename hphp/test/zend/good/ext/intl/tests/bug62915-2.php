<?hh
class A extends IntlDateFormatter {
        static $ARGS = array("en_US" ,IntlDateFormatter::FULL, IntlDateFormatter::FULL,
            'America/Los_Angeles', IntlDateFormatter::GREGORIAN);
}
class B extends NumberFormatter {
        static $ARGS = array('de_DE', NumberFormatter::DECIMAL);
}
class C extends MessageFormatter {
        static $ARGS = array("en_US", "foo");
}
class D extends Spoofchecker {
        static $ARGS = array();
}
<<__EntryPoint>> function main(): void {
foreach (range('A', 'D') as $subclass) {
        $rc = new ReflectionClass($subclass);
            $obj = $rc->newInstanceArgs($subclass::$ARGS);
                $clone = clone $obj;
                    var_dump(get_class($clone));
}
}
