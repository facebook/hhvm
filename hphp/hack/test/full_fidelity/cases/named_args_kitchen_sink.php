<?hh
<<file:__EnableUnstableFeatures('named_parameters_use')>>

function main(): void {
    // Basic named argument calls - following the exact pattern from named_args_enabled.php
    test_named_args(1, the_arg_name = 2);
    test_named_args(1, 2, another_arg = 3);
    test_named_args(positional_arg = 1, second_arg = 2);

    // Multiple named arguments
    multi_arg_function(first = 1, second = "hello", third = true);
    multi_arg_function(third = false, first = 10, second = "world");

    // Mixed positional and named
    mixed_function(1, 2, named_param = "test");
    mixed_function(1, named_param = "example", another_named = 42);

    // Constructor calls with named arguments
    $obj = new SomeClass(regular_param = 1, other_param = "value");

    // Method calls with named arguments
    $obj->some_method(param_name = "test", count = 5);

    // Static method calls
    SomeClass::static_method(arg1 = 1, arg2 = 2);

    // Nested function calls with named arguments
    outer_function(inner_function(param = 1), other_param = 2);

    // Named arguments with various value types
    type_examples(
        int_param = 42,
        string_param = "hello",
        bool_param = true,
        float_param = 3.14,
        null_param = null
    );

    // Named arguments with expressions
    expression_test(
        calculated = 1 + 2 * 3,
        string_concat = "hello" . " world",
        function_result = strlen("test")
    );

    foo(param=1);
    bar(x=10, y=20);
    test_call(name="hello", count=42);
    mixed_call(good=1, bad=2, also_good=3);
    complex_test(result=calculate(10), flag=true, data=vec[1,2,3]);
    nested(outer=inner(param=1), other=func(x=2));
    $obj = new TestClass(prop=99, name="test");
    $obj->method(param=123);
    StaticClass::method(arg=456);
    $obj->first(a=1)->second(b=2)->third(c=3);
    $lambda = (int $x) ==> $x * 2;
    lambda_test(callback=$lambda, value=5, apply=true);
    array_test(data=vec[1, 2, 3], map=dict['a' => 1]);
    string_test(template="Hello {$name}", name="World");
    conditional_test(value=true ? 1 : 0, label=false ? "no" : "yes");

    multi_test(
        good = 1,
        bad=2,
        also_good = "test",
        also_bad=true
    );

    compound_expressions(
        comparison=($a == $b),
        arithmetic=(1 + 2 * 3),
        function_call=strlen("test"),
        array_access=($arr[0]),
        property_access=($obj->prop),
        ternary=($cond ? $a : $b),
        lambda_with_assignment=(() ==> { $x = 5; return $x; }),
        closure_assignment=(function() { $x = 5; return $x; }),
    );

    // Test mixing positional, named, and splat arguments
    // see also ./named_args_splat_before_named.php
    $args = vec[1, 2, 3];
    mixed_test(42, name="test", flag=true, ...$args);
    pos_named_splat(100, option=false, count=5, ...$values);
    named_then_splat(start=0, middle="x", end=99, ...$values);
    tuple_mix(prefix="start", suffix="end", debug=false, ...$values);
    many_named_splat(first=1, second=2, third=3, flag=true, ...$values);
}
