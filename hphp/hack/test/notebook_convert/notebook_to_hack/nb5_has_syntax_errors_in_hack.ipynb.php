<?hh
// copyright header here
//@bento-notebook:{name: "n1234"}
//@bento-cell:{"id": 2, "cell_type": "code"}
class MyClass {}


function notebook_main_n1234(): void {
    //@bento-cell:{"id": 1, "cell_type": "code"}
    $x = new MyClass();
    // The next line is not valid Hack:
    =$x;
}
