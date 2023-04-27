<?hh

class UserAttribute implements HH\ClassAttribute {
    public function __construct(mixed $x) {}
}

<<UserAttribute(self::class)>>
class SomeClass {}
