<?hh

//@bento-cell:{"id": 2, "cell_type": "markdown"}
/*
# Check it out

I am a *markdown* **cell**
*/

//@bento-cell:{"id": 1, "cell_type": "code"}
class MyClass {
    public function hello(): void {
        echo "hello";
    }
}


function notebook_main_n1234(): void {
    //@bento-cell:{"id": 1, "cell_type": "code"}
    $m = new MyClass();
    echo "hi1";
    //@bento-cell:{"id": 3, "cell_type": "code"}
    echo "hi2";
    $m->hello();
}
