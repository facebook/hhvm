<?hh
// copyright: the company
//@bento-notebook:{"notebook_number":"1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}

//@bento-cell:{"id": 2, "cell_type": "markdown"}
/*@non_hack:
# Check it out

I am a *markdown* **cell**
*/
//@bento-cell-end

//@bento-cell:{"id": 1, "cell_type": "code"}
class N1234MyClass {
  public function hello(): void {
    echo "hello";
  }
}
//@bento-cell-end
async function gen_n1234_notebook_main(): Awaitable<void> {
  //@bento-cell:{"cell_bento_metadata":{"output": {"id":1247934846418027,"loadingStatus":"loaded"}, "collapsed": true},"cell_type":"code","id":1}
  $m = new N1234MyClass();
  echo "hi1";
  //@bento-cell-end
  //@bento-cell:{"id": 3, "cell_type": "code"}
  echo "hi2";
  $m->hello();
  //@bento-cell-end
}
