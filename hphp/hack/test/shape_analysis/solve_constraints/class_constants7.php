<?hh
// T134611333
class C {
    const dict<string, mixed> DICT = dict["a" => 2];


}

class D extends C {
    const dict<string, mixed> DICT = dict["b" => "apple"];
}
