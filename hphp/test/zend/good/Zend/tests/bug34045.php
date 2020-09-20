<?hh
class BasicSingleton
{
    private static $instance;

    public function __wakeup() {
        self::$instance = $this;
    }

    public static function singleton() {
        if (!(self::$instance is BasicSingleton)) {
            $c = __CLASS__;
            self::$instance = new $c;
        }
        return self::$instance;
    }
}
<<__EntryPoint>> function main(): void {
$db = BasicSingleton::singleton();
$db_str = serialize($db);
$db2 = unserialize($db_str);
echo "ok\n";
}
