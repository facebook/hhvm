<?hh

// class without methods, without any members, only static props
class A {
    public  static vec<string>  $pub_sprop_static = vec[];
    public  static string  $pub_sprop_string;
    public  static int  $pub_sprop_int;
}

class B extends A {
    public  static vec<string>  $pub_sprop_static = vec[];
}

function check_and_print($condition, $message) {
    if ($condition) {
        echo "Pass";
    } else {
        echo "Fail";
    }
    echo ": " . $message . "\n";
}

<<__EntryPoint>>
function main(): void {
    A::$pub_sprop_static[] = str_repeat('a', 100);
    A::$pub_sprop_static[] = str_repeat('b', 100);
    A::$pub_sprop_static[] = str_repeat('c', 100);

    A::$pub_sprop_string = str_repeat('d', 100);

    A::$pub_sprop_int = 69;
    B::$pub_sprop_static[] = str_repeat('f', 100);

    $prof = objprof_get_data_extended();
    $prof_per_prop = objprof_get_data_extended(OBJPROF_FLAGS_PER_PROPERTY);

    // Instance count of everything should be 1, because static
    check_and_print($prof["A::SPropCache"]["instances"] == 1, "Instance count of A::SPropCache should be 1");
    check_and_print($prof["B::SPropCache"]["instances"] == 1, "Instance count of B::SPropCache should be 1");
    check_and_print($prof_per_prop["A::pub_sprop_static"]["instances"] == 1, "Static prop instance count should be 1");
    check_and_print($prof_per_prop["B::pub_sprop_static"]["instances"] == 1, "Static prop instance count should be 1");
    check_and_print($prof_per_prop["A::pub_sprop_string"]["instances"] == 1, "Static prop instance count should be 1");
    check_and_print($prof_per_prop["A::pub_sprop_int"]["instances"] == 1, "Static prop instance count should be 1");

    check_and_print($prof["A::SPropCache"]["bytes_normalized"] > 400, "The normalized size of A should be greater than 400");
    check_and_print($prof["B::SPropCache"]["bytes_normalized"] > 100, "The normalized size of B should be greater than 100");

    check_and_print($prof_per_prop["A::pub_sprop_static"]["bytes_normalized"] > 300, "The normalized size of A::pub_sprop_static should be greater than 300");
    check_and_print($prof_per_prop["B::pub_sprop_static"]["bytes_normalized"] > 100, "The normalized size of B::pub_sprop_static should be greater than 100");
    check_and_print($prof_per_prop["A::pub_sprop_string"]["bytes_normalized"] > 100, "The normalized size of A::pub_sprop_static should be greater than 100");
    check_and_print($prof_per_prop["A::pub_sprop_int"]["bytes_normalized"] > 8, "The normalized size of A::pub_sprop_int should be greater than 8");

}
