<?hh
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}

// Our Jupyter notebooks implementation cannot handle an empty cell list,
// so we add an empty cell if the user's notebook has no cells.
function gen_N1234_notebook_main(): Awaitable<void> {}
