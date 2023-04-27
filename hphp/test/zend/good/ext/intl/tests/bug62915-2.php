<?hh
class A extends IntlDateFormatter {
        public static $ARGS = varray["en_US" ,IntlDateFormatter::FULL, IntlDateFormatter::FULL,
            'America/Los_Angeles', IntlDateFormatter::GREGORIAN];
}
class B extends NumberFormatter {
        public static $ARGS = varray['de_DE', NumberFormatter::DECIMAL];
}
class C extends MessageFormatter {
        public static $ARGS = varray["en_US", "foo"];
}
class D extends SpoofChecker {
        public static $ARGS = varray[];
}
<<__EntryPoint>> function main(): void {
foreach (range('A', 'D') as $subclass) {
        $rc = new ReflectionClass($subclass);
            $obj = $rc->newInstanceArgs($subclass::$ARGS);
                $clone = clone $obj;
                    var_dump(get_class($clone));
}
}
