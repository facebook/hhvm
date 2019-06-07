<?hh

trait T {
    function wat() {
        var_dump(self::class);
    }
}

class F { use T; }
class G { use T; }
class H extends G {}


<<__EntryPoint>>
function main_self_class_name() {
(new F)->wat();
(new G)->wat();
(new H)->wat();
}
