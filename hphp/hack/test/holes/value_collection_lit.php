<?hh

 function value_collection_lit():keyset<int> {
   /* HH_FIXME[4110] */
   return keyset [ 'bad' , 1 ];
 }
