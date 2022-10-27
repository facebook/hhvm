<?hh

class C {
    const dict<string, mixed> DICT = dict["a" => 2];
    // type incorrectly solved to: shape('a' => int)
    // should be                 : shape(?"a": int, ?b: string)
}

class D extends C {
    const dict<string, mixed> DICT = dict["b" => "apple"];
}
