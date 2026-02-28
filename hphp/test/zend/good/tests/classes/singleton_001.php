<?hh

class Counter {
    private $counter = 0;

    function increment_and_print() :mixed{
        echo ++$this->counter;
        echo "\n";
    }
}


class SingletonCounter {
    private static $m_instance = NULL;

    static function Instance() :mixed{
        if (self::$m_instance == NULL) {
            self::$m_instance = new Counter();
        }
        return self::$m_instance;
    }
}
<<__EntryPoint>> function main(): void {
SingletonCounter::Instance()->increment_and_print();
SingletonCounter::Instance()->increment_and_print();
SingletonCounter::Instance()->increment_and_print();
}
