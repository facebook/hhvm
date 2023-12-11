<?hh
const FIRST_KEY = "a";
const THIRD_KEY = "c";


function test()
:mixed{
        $arr = dict[
                FIRST_KEY => "111",
                "b" => "222",
                THIRD_KEY => "333",
                "d" => "444"
        ];
        print_r($arr);
}

function test2()
:mixed{
        $arr = dict[
                FIRST_KEY => "111",
                "a" => "222",
                "c" => "333",
                THIRD_KEY => "444"
        ];
        print_r($arr);
}
<<__EntryPoint>> function main(): void {
test();
test2();
}
