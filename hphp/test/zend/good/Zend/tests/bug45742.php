<?hh
class Constants {
    // Needs to be equal
    const A = 1;
    const B = 1;
}

class ArrayProperty {
    public static $array = dict[
        Constants::A => 23,
        Constants::B => 42,
    ];
}
<<__EntryPoint>> function main(): void {
var_dump( ArrayProperty::$array );
}
