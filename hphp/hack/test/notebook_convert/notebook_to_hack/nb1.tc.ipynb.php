<?hh
// copyright header here
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}
//@bento-cell:{"cell_bento_metadata":{"collapsed":true,"output":{"id":1247934846418027,"loadingStatus":"loaded"}},"cell_type":"code","id":1}
/* Very classy */
class MyClass {}
//@bento-cell-end

//@bento-cell:{"cell_bento_metadata":{},"cell_type":"markdown","id":3}
/*
## Markdown cell (cell 3)

I am a *markdown* **cell**
*/
//@bento-cell-end

async function gen_notebook_main_N1234(): Awaitable<void> {
  //@bento-cell:{"cell_bento_metadata":{"collapsed":true,"output":{"id":1247934846418027,"loadingStatus":"loaded"}},"cell_type":"code","id":1}
  echo "hi from cell 1";
  //@bento-cell-end
  //@bento-cell:{"cell_bento_metadata":{"collapsed":true,"output":{"id":1247934846418027,"loadingStatus":"loaded"}},"cell_type":"code","id":1}
  // I am a comment
  echo "end of cell 1";
  //@bento-cell-end
  //@bento-cell:{"cell_bento_metadata":{},"cell_type":"code","id":2}
  echo "hi from cell 2";
  $x = 3;
  echo $x + 1;
  echo "end of cell 2";
  //@bento-cell-end
  //@bento-cell:{"cell_bento_metadata":{},"cell_type":"code","id":4}
  echo "this is cell 4";
  await (async () ==> {})();
  //@bento-cell-end
}
