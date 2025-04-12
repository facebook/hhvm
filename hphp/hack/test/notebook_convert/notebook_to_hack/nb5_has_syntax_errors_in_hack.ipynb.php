<?hh
// copyright header here
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}
//@bento-cell:{"cell_bento_metadata":{"collapsed":true},"cell_type":"code","id":1}
class N1234MyClass {}
//@bento-cell-end


async function gen_n1234_notebook_main(): Awaitable<void> {
    //@bento-cell:{"cell_bento_metadata":{},"cell_type":"code","id":2}
    $x = new N1234MyClass();
    // The next line is not valid Hack:
    =$x;
    //@bento-cell-end
}

