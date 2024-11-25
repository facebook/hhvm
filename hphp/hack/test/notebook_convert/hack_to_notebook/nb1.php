<?hh
// copyright: the company
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}

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

async function gen_notebook_main_n1234(): Awaitable<void> {
  //@bento-cell:{"cell_bento_metadata":{"output": {"id":1247934846418027,"loadingStatus":"loaded"}, "collapsed": true},"cell_type":"code","id":1}
  $m = new MyClass();
  echo "hi1";
  //@bento-cell:{"id": 3, "cell_type": "code"}
  echo "hi2";
  $m->hello();
}
