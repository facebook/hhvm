<?hh
// copyright header here
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}
//@bento-cell:{"cell_bento_metadata":{"collapsed":true,"output":{"id":1247934846418027,"loadingStatus":"loaded"}},"cell_type":"code","id":1}
/* Very classy */
class N1234MyClass {}
//@bento-cell-end

//@bento-cell:{"cell_bento_metadata":{"language":"sql"},"cell_type":"code","id":3}
/*@non_hack:
SELECT
    col1,
    col2
FROM the_table
WHERE
    col1 = 0
    AND col2 IS NOT NULL
LIMIT
    5
*/
//@bento-cell-end

//@bento-cell:{"cell_type":"code","id":4}
/*@non_hack:
%%python
print('hello from python', {'python': 'dictionary'})
print('no semicolons')
*/
//@bento-cell-end

async function gen_n1234_notebook_main(): Awaitable<void> {
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
}
