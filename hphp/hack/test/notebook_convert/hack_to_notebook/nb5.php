<?hh
// copyright: the company
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}
//@bento-cell:{"id": 1, "cell_type": "markdown"}
/*
test another markdown
*/
//@bento-cell-end

//@bento-cell:{"id": 3, "cell_type": "markdown"}
/*
test this is markdown
*/
//@bento-cell-end

async function gen_n1234_notebook_main_(): Awaitable<void> {
  //@bento-cell:{"id": 2, "cell_type": "code"}
  $a = 10;
  //@bento-cell-end
  //@bento-cell:{"id": 4, "cell_type": "code"}
  $vc = await gen_myvc();
  //@bento-cell-end
}
