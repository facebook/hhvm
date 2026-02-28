<?hh

function foo(): /*range-start*/shape('a' => int, ...)/*range-end*/ {
    return shape('a' => 1);
}
