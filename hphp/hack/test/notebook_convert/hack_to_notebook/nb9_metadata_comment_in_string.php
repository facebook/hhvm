<?hh
// copyright: the company
//@bento-notebook:{"notebook_number":"N1234","kernelspec":{"display_name":"hack","language":"hack","name":"bento_kernel_hack"}}

async function gen_n1234_notebook_main(): Awaitable<void> {
  //@bento-cell:{"cell_bento_metadata":{"output": {"id":1247934846418027,"loadingStatus":"loaded"}, "collapsed": true},"cell_type":"code","id":1}
  $m = new MyClass();
  echo "hi1";
  //@bento-cell-end
  // Something that *looks* like a cell marker but is part of a string will break the converter.
  // This is the price we pay for allowing ill-formed Hack code in Bento notebooks:
  // the hack->notebook converter doesn't read a Hack file *as* Hack, it reads the file
  // as a collection of cells baseed on the magic @bento-cell markers and @bento-cell-end markers.
  $s = "
  //@bento-cell:adsfdsafdsafds
  ";
  echo "hi2";
  $m->hello();
  //@bento-cell-end

}
