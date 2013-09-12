# Zend Extension Source Compatability Layer

If you want to compile your existing zend extension against HHVM, you can use 
these headers. The runtimes are similar enough that we can just map the macros 
to our data structures and it mostly works. 

## Migration Steps

  cp -R <zend_extension_dir> runtime/ext_zend_compat/<ext_name>
  
  # move all the .c to .cpp
  for i in runtime/ext_zend_compat/<ext_name>/*.c; do mv $i "$i"pp; done 

  # If your extension has docs on php.net you can make the idl like this:
  cd system/idl
  php newexp.php <ext_name>
  # Otherwise you have to make the .idl by hand

  <setup the build environment>

## Things you have to fix in your code

* C++ compile errors
* Use `Z_RESVAL` instead of `Z_LVAL` for resource access
