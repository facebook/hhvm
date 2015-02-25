<?php

/*
 * Measures and reports on the relative cost of basic HHVM operations.
 * The goal is to provide insight into the perf costs of simple constructs to
 * aid early design choices for writing performant PHP/Hack.
 * Run it via './hhmeasure (or ./hhmeasure basic)'.
 */


// Include microbenchmark code
require(dirname(__FILE__)."/BasicOps.php");

function mean($array) {
  return (array_sum($array) / count($array));
}

function median($array) {
  rsort($array);
  $mid = round(count($array) / 2);
  $median = $array[$mid-1];
  return ($median);
}

function std_dev_square($x, $mean) {
  return pow($x - $mean,2);
}

function std_dev($array) {
  return sqrt(array_sum(array_map("std_dev_square", $array, array_fill(0,
    count($array), mean($array)))) / (count($array)-1));
}

class TimeHHOperations {
  public static $lowconfidenceCount = 0;
  public static $operationsCount = 0;
  public static $nmedian = 1;
  public static $file;
  public static function openfile() {
    TimeHHOperations::$file = fopen(dirname(__FILE__)."/hhmeasure.csv", 'w');
  }
  public static function closefile() {
    fclose (TimeHHOperations::$file);
  }
  public static function timeop($opname, $time_function) {
    // Pick sample size, results normalized against median function call cost
    $run_count = 10;

    // Run operation a few times before starting timer
    $result = $time_function(20);
    echo "\n{$opname} result = {$result}";

    // Record time
    $time = array();
    for ($iter = 0; $iter < $run_count; $iter++) {
      $start_time = microtime(true);
      $result = $time_function(10000);
      $end_time = microtime(true);
      if ($iter == 0) {
      echo "\n{$opname} result = {$result}";
      }
      $time[$iter] = ($end_time - $start_time) * 1000;
    }

    // Compute stats
    $max = max($time);
    $min = min($time);
    $mean = mean($time);
    $median = median($time);
    $std_dev = std_dev($time);

    // Confidence in results
    TimeHHOperations::$operationsCount++;
    if (($max > 10 * $min) || ($std_dev / $mean > 0.1)) {
      TimeHHOperations::$lowconfidenceCount++;
    }

    if ($opname == 'function_call') {
      TimeHHOperations::$nmedian = $median;
    }

    // Print stats
    echo "\nCPU cost normalized against median function call cost";
    echo "\n-----------------------------------------------------";
    echo "\n{$opname} max = ", ($max / TimeHHOperations::$nmedian);
    echo "\n{$opname} min = ", ($min / TimeHHOperations::$nmedian);
    echo "\n{$opname} mean = ", ($mean / TimeHHOperations::$nmedian);
    echo "\n{$opname} median = ", ($median / TimeHHOperations::$nmedian);
    echo "\n{$opname} std. dev = {$std_dev}";
    echo "\n{$opname} time measurement complete. Run count = {$run_count}.";
    echo "\n";
    // Write stats to csv file
    fputcsv(TimeHhOperations::$file, array($opname,
    $max / TimeHHOperations::$nmedian, $min / TimeHHOperations::$nmedian,
    $mean / TimeHHOperations::$nmedian, $median / TimeHHOperations::$nmedian));
  }
}

class Runner {
  public static function openfile() {
    TimeHHOperations::openfile();
  }

  // Add microbenchmarks to BasicOps.php and add calls to them here
  public static function runtestsBasic() {
    /* 0: Do nothing */
    $loop_do_nothing = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
      }
      return ($result);
    };
    TimeHHOperations::timeop('nothing', $loop_do_nothing);

    /* 1: Function call */
    $loop_function_call = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
        $result += function_call(0);
      }
      return ($result);
    };
    TimeHHOperations::timeop('function_call', $loop_function_call);

    /* 2: Method call */
    $loop_static_method_call = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
        $result += MethodCall::callStaticMethod(0);
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('static_method_call', $loop_static_method_call);

    $loop_instance_method_call = function ($count) {
      $result = 0;
      $instance = new MethodCall();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->callInstanceMethod(0);
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('instance_method_call', $loop_instance_method_call);

    $loop_interface_method_call = function ($count) {
      $result = 0;
      $instance = new Class0();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->callInterfaceMethod(0);
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('interface_method_call', $loop_interface_method_call);

    /* 3: IndexArrayMap */
    $loop_idxemptyarray = function ($count) {
      $result = 0;
      $instance = new EmptyArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->indexEmpty();
      }
      return ($result);
    };
    TimeHHOperations::timeop('idxemptyarray', $loop_idxemptyarray);

    $loop_idxnohit_smallarray = function ($count) {
      $result = 0;
      $instance = new NoHitSmallArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->indexNoHitSmall();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('idxnohit_smallarray', $loop_idxnohit_smallarray);

    $loop_idxhit_smallarray = function ($count) {
      $result = 0;
      $instance = new HitSmallArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->indexHitSmall();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('idxhit_smallarray', $loop_idxhit_smallarray);

    $loop_idxnohit_largearray = function ($count) {
      $result = 0;
      $instance = new NoHitLargeArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->indexNoHitLarge();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('idxnohit_largearray', $loop_idxnohit_largearray);

    $loop_idxhit_largearray = function ($count) {
      $result = 0;
      $instance = new HitLargeArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->indexHitLarge();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('idxhit_largearray', $loop_idxhit_largearray);

    /* 4: MethodExists */
    $loop_methodexists_string = function ($count) {
      $result = 0;
      $instance = new MethodExistsString();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->methodexists();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('methodexists_string', $loop_methodexists_string);

    $loop_methodexists = function ($count) {
      $result = 0;
      $instance = new MethodExistsClass();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->methodexists();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('methodexists', $loop_methodexists);

    $loop_methodnotexists = function ($count) {
      $result = 0;
      $instance = new MethodNotExistsClass();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->methodexists();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('methodnotexists', $loop_methodnotexists);

    $loop_methodexists_base = function ($count) {
      $result = 0;
      $instance = new MethodExistsDerivedClass();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->methodexists();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('methodexists_base', $loop_methodexists_base);

    $loop_methodnotexists_base = function ($count) {
      $result = 0;
      $instance = new MethodNotExistsDerivedClass();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->methodexists();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('methodnotexists_base', $loop_methodnotexists_base);

    /* 5: Field Access */
    $loop_fieldaccess_staticint = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
        $result += FieldAccess::accessStaticCounter();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('fieldaccess_staticint', $loop_fieldaccess_staticint);

    $loop_fieldaccess_instanceint = function ($count) {
      $result = 0;
      $instance = new FieldAccess();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->accessInstanceCounter();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('fieldaccess_instanceint', $loop_fieldaccess_instanceint);

    $loop_fieldaccess_staticstring = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
        $result += FieldAccess::accessStaticString();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('fieldaccess_staticstring', $loop_fieldaccess_staticstring);

    $loop_fieldaccess_instancestring = function ($count) {
      $result = 0;
      $instance = new FieldAccess();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->accessInstanceString();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('fieldaccess_instancestring', $loop_fieldaccess_instancestring);

    /* 6: Array Access */
    $loop_arrayassign_int = function ($count) {
      $result = 0;
      $instance = new IntArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->assignArrayElem();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('arrayassign_int', $loop_arrayassign_int);

    $loop_arrayassign_string = function ($count) {
      $result = 0;
      $instance = new StringArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->assignArrayElem();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('arrayassign_string', $loop_arrayassign_string);

    /* 7: Closures et al */
    $loop_anonymousfunc_instance = function ($count) {
      $result = 0;
      $instance = new AnonymousFunctions();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->instanceFunction();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('anonymousfunc_instance', $loop_anonymousfunc_instance);

    $loop_anonymousfunc_static = function ($count) {
      $result = 0;
      $instance = new AnonymousFunctions();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += AnonymousFunctions::staticFunction();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('anonymousfunc_static', $loop_anonymousfunc_static);

    $loop_variablefunc_instance = function ($count) {
      $result = 0;
      $instance = new AnonymousFunctions();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->variableFunction();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('variablefunc_instance', $loop_variablefunc_instance);

    $loop_variablefunc_static = function ($count) {
      $result = 0;
      $instance = new AnonymousFunctions();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += AnonymousFunctions::variableFunctionStatic();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('variablefunc_static', $loop_variablefunc_static);

  }

  public static function runtestsInstanceof() {
    $loop_instanceof_miss0 = function ($count) {
      $result = 0;
      $instance = new Miss0();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->instanceofMiss0();
      }
      return ($result);
    };
    TimeHHOperations::timeop('instanceof_miss0', $loop_instanceof_miss0);

    $loop_instanceof_miss1 = function ($count) {
      $result = 0;
      $instance = new Miss1();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->instanceofMiss1();
      }
      return ($result);
    };
    TimeHHOperations::timeop('instanceof_miss1', $loop_instanceof_miss1);

    $loop_instanceof_hit1 = function ($count) {
      $result = 0;
      $instance = new Hit1();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->instanceofHit1();
      }
      return ($result);
    };
    TimeHHOperations::timeop('instanceof_hit1', $loop_instanceof_hit1);

    $loop_instanceof_miss4 = function ($count) {
      $result = 0;
      $instance = new Miss4();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->instanceofMiss4();
      }
      return ($result);
    };
    TimeHHOperations::timeop('instanceof_miss4', $loop_instanceof_miss4);

    $loop_instanceof_hit5 = function ($count) {
      $result = 0;
      $instance = new Hit5();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->instanceofHit5();
      }
      return ($result);
    };
    TimeHHOperations::timeop('instanceof_hit5', $loop_instanceof_hit5);
  }

  public static function runtestsIsset() {
    $loop_issetcheck = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
        $result += X::isX();
      }
      return ($result);
    };
    TimeHHOperations::timeop('issetcheck', $loop_issetcheck);

    $loop_noissetcheck = function ($count) {
      $result = 0;
      for ($iter = 0; $iter < $count; $iter++) {
        $result += Y::isY();
      }
      return ($result);
    };
    TimeHHOperations::timeop('noissetcheck', $loop_noissetcheck);
  }

  public static function runtestsIteration() {
    $loop_iterate_sum20 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->sumNum20();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_sum20', $loop_iterate_sum20);

    $loop_iterate_sum100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->sumNum100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_sum100', $loop_iterate_sum100);

    $loop_iterate_intarray10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntArray10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intarray_foreach10', $loop_iterate_intarray10);

    $loop_iterate_intarrayfor10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntArrayFor10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intarray_for10', $loop_iterate_intarrayfor10);

    $loop_iterate_stringarray10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringArray10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringarray_foreach10', $loop_iterate_stringarray10);

    $loop_iterate_stringarrayfor10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringArrayFor10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringarray_for10', $loop_iterate_stringarrayfor10);

    $loop_iterate_objectarray10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectArray10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectarray_foreach10', $loop_iterate_objectarray10);

    $loop_iterate_objectarrayfor10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectArrayFor10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectarray_for10', $loop_iterate_objectarrayfor10);

    $loop_iterate_intarray100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntArray100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intarray_foreach100', $loop_iterate_intarray100);

    $loop_iterate_intarrayfor100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntArrayFor100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intarray_for100', $loop_iterate_intarrayfor100);

    $loop_iterate_stringarray100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringArray100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringarray_foreach100', $loop_iterate_stringarray100);

    $loop_iterate_stringarrayfor100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringArrayFor100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringarray_for100', $loop_iterate_stringarrayfor100);

    $loop_iterate_objectarray100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectArray100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectarray_foreach100', $loop_iterate_objectarray100);

    $loop_iterate_objectarrayfor100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestArray();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectArrayFor100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectarray_for100', $loop_iterate_objectarrayfor100);

    $loop_iterate_intvector10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntVector10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intvector_foreach10', $loop_iterate_intvector10);

    $loop_iterate_intvectorfor10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntVectorFor10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intvector_for10', $loop_iterate_intvectorfor10);

    $loop_iterate_stringvector10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringVector10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringvector_foreach10', $loop_iterate_stringvector10);

    $loop_iterate_stringvectorfor10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringVectorFor10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringvector_for10', $loop_iterate_stringvectorfor10);

    $loop_iterate_objectvector10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectVector10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectvector_foreach10', $loop_iterate_objectvector10);

    $loop_iterate_objectvectorfor10 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectVectorFor10();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectvector_for10', $loop_iterate_objectvectorfor10);

    $loop_iterate_intvector100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntVector100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intvector_foreach100', $loop_iterate_intvector100);

    $loop_iterate_intvectorfor100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateIntVectorFor100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_intvector_for100', $loop_iterate_intvectorfor100);

    $loop_iterate_stringvector100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringVector100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringvector_foreach100', $loop_iterate_stringvector100);

    $loop_iterate_stringvectorfor100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateStringVectorFor100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_stringvector_for100', $loop_iterate_stringvectorfor100);

    $loop_iterate_objectvector100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectVector100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectvector_foreach100', $loop_iterate_objectvector100);

    $loop_iterate_objectvectorfor100 = function ($count) {
      $result = 0;
      $instance = new IteratorTestVector();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->iterateObjectVectorFor100();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('iterate_objectvector_for100', $loop_iterate_objectvectorfor100);
  }

  public static function runtestsRegex() {
    $loop_regex_pcre_len18 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexPCRELen18();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_pcre_len18', $loop_regex_pcre_len18);

    $loop_regex_posix_len18 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexPOSIXLen18();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_posix_len18', $loop_regex_posix_len18);

    $loop_regex_byhand_len18 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexByHandLen18();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_byhand_len18', $loop_regex_byhand_len18);

    $loop_regex_pcre_len85 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexPCRELen85();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_pcre_len85', $loop_regex_pcre_len85);

    $loop_regex_posix_len85 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexPOSIXLen85();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_posix_len85', $loop_regex_posix_len85);

    $loop_regex_byhand_len85 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexByHandLen85();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_byhand_len85', $loop_regex_byhand_len85);

    $loop_regex_pcre_len152 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexPCRELen152();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_pcre_len152', $loop_regex_pcre_len152);

    $loop_regex_posix_len152 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexPOSIXLen152();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_posix_len152', $loop_regex_posix_len152);

    $loop_regex_byhand_len152 = function ($count) {
      $result = 0;
      $instance = new RegEx();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regexByHandLen152();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regex_byhand_len152', $loop_regex_byhand_len152);
  }

  public static function runtestsReflection() {
    $loop_reflection_getobjclassname = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->reflectionGetObjectClassName();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('reflection_getobjclassname', $loop_reflection_getobjclassname);

    $loop_regular_gettype = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regularGetType();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regular_gettype', $loop_regular_gettype);

    $loop_refinvoke_emptystatic = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->reflectionInvokeEmptyStatic();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('refinvoke_emptystatic', $loop_refinvoke_emptystatic);

    $loop_refinvoke_emptystatic5Arg = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->reflectionInvokeEmptyStatic5Arg();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('refinvoke_emptystatic5Arg', $loop_refinvoke_emptystatic5Arg);

    $loop_refinvokeargs_emptystatic5Arg = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->reflectionInvokeArgsEmptyStatic5Arg();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('refinvokeargs_emptystatic5Arg',
       $loop_refinvokeargs_emptystatic5Arg);

    $loop_regular_emptystaticmethodcall = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regularEmptyStaticMethodCall();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regular_emptystaticmethodcall',
        $loop_regular_emptystaticmethodcall);

    $loop_regular_emptystaticmethod5argcall = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regularEmptyStaticMethod5ArgCall();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regular_emptystaticmethod_5args_call',
        $loop_regular_emptystaticmethod5argcall);

    $loop_reflection_getinstanceintprop = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->reflectionGetInstanceIntProp();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('reflection_getinstanceintprop',
        $loop_reflection_getinstanceintprop);

    $loop_reflection_setinstanceintprop = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->reflectionSetInstanceIntProp();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('reflection_setinstanceintprop',
        $loop_reflection_setinstanceintprop);

     $loop_regular_setinstanceintprop = function ($count) {
      $result = 0;
      $instance = new ReflectionTest();
      for ($iter = 0; $iter < $count; $iter++) {
        $result += $instance->regularSetInstanceIntProp();
      }
      return ($result);
    };
    TimeHHOperations::
      timeop('regular_setinstanceintprop',
        $loop_regular_setinstanceintprop);
  }

  public static function closefile() {
    TimeHHOperations::closefile();
  }

  public static function reportresults () {
    $array = file(dirname(__FILE__)."/hhmeasure.csv");
    foreach ($array as $key => $var) {
      $array[$key] = explode(',', $var);
    }
    echo "\nSummary: CPU costs normalized against median function call cost";
    echo "\nPlease see hhmeasure.csv to import results to a spreadsheet.";
    echo "\n---------------------------------------------------------------";
    if ((TimeHHOperations::$lowconfidenceCount /
         TimeHHOperations::$operationsCount) >= 0.5) {
      echo "\nLow confidence in results. Too many other processes running?";
    }
    echo sprintf("\n%-48s%-16s%-16s%-16s%-16s",
      "Operation Name", "Median", "Mean", "Max", "Min");
    foreach ($array as $key => $value) {
     echo sprintf("\n%-40s", ($array[$key][0]));
     echo sprintf("% 16.6f", ($array[$key][4]));
     echo sprintf("% 16.6f", ($array[$key][3]));
     echo sprintf("% 16.6f", ($array[$key][1]));
     echo sprintf("% 16.6f", ($array[$key][2]));
    }
    echo "\n";
    $array = file("/proc/cpuinfo");
    foreach ($array as $key => $var) {
      $array[$key] = array_map('trim', explode(':', $var));
    }
    $processorcount = 0;
    $processormodel = '';
    $processorspeed = '';
    foreach ($array as $var) {
      if ($var[0] == 'model name') {
        $processormodel = $var[1];
        $processorcount++;
      }
      if ($var[0] == 'cpu MHz') {
        $processorspeed = $var[1];
      }
    }

    // Memory info
    $array = file("/proc/meminfo");
    foreach ($array as $key => $var) {
      $array[$key] = array_map('trim', explode(':', $var));
    }
    $memsize = '';
    foreach ($array as $var) {
      if ($var[0] == 'MemTotal') {
        $memsize = $var[1];
      }
    }

    // Cache info
    $cachefiles = array();
    $dirpath = '/sys/devices/system/cpu/cpu0/cache/';
    $dirhandle = opendir($dirpath);
    $count = 0;
    while (($entry = readdir($dirhandle)) !== false) {
      if ($entry != "." && $entry != "..") {
        $cachefiles[$count] = $entry;
        $count++;
      }
    }
    closedir($dirhandle);
    $num_caches = $count;
    $cachesizes = array();
    $cachetypes = array();
    for ($count = 0 ; $count < $num_caches; $count++) {
      $fh_size = fopen($dirpath . $cachefiles[$count] . "/size", "r");
      $fh_type = fopen($dirpath . $cachefiles[$count] . "/type", "r");
      $cachesizes[$count] = trim(fgets($fh_size), "\n");
      $cachetypes[$count] = trim(fgets($fh_type), "\n");
      fclose($fh_size);
      fclose($fh_type);
    }

    echo "\nAttributes of the machine used to collect the data";
    echo "\n--------------------------------------------------";
    echo "\nComputer Name\t\t\t";
    system('hostname');
    echo "Number of Processors\t\t" . "{$processorcount}";
    echo "\nProcessor Model\t\t\t" . "{$processormodel}";
    echo "\nProcessor MHz\t\t\t" . "{$processorspeed}";
    echo "\nCache Types and Sizes";
    echo
      sprintf("%10s %-16s %-16s\n",
      (""), ($cachetypes[0]), ($cachesizes[0]));
    for ($count = 1 ; $count < $num_caches ; $count++) {
       echo
       sprintf("%31s %-16s %-16s\n",
        (""), ($cachetypes[$count]), ($cachesizes[$count]));
    }
    echo "Memory \t\t\t\t" . "{$memsize}";
    echo "\nOperating System\t\t";
    system('cat /etc/system-release');
    echo "OS Version\t\t\t";
    system('uname -mrs');
    echo "Compile Type\t\t\t" .
      "JIT-compiled, repo authoritative mode, inlining turned off";
    echo "\n";
    echo "\n";
  }
}

Runner::openfile();
if ($argv[1] == 'basic') {
  Runner::runtestsBasic();
}
else {
  Runner::runtestsBasic();
  Runner::runtestsInstanceof();
  Runner::runtestsIsset();
  Runner::runtestsIteration();
  Runner::runtestsRegex();
  Runner::runtestsReflection();
}
Runner::closefile();
Runner::reportresults();
