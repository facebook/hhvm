<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

type simple = (function<T>(T): T);

type defParamAsRet<TOut> = (function<TIn>(TIn): TOut);

type defParamsAsBound<TSub, TSuper> = (function<TIn as TSub super TSuper>(TIn): TIn);

type defWithUserAttr = (function<<<__NoAutoBound>> T>(T): T);



function param_pos((function<T>(T):T) $_): void {}

function param_pos_fn_param_as_ret<TOut>((function<T>(T):TOut) $_): void {}

function param_pos_fn_param_as_bound<TSub,TSuper>((function<T as TSub super TSuper>(T):T) $_): void {}

function param_pos_with_user_attr((function<<<__NoAutoBound>> T>(T):T) $_): void {}



function ret_pos(): (function<T>(T):T) {
  throw new Exception();
}

function ret_pos_fn_param_as_ret<TOut>(): (function<T>(T):TOut) {
  throw new Exception();
}

function ret_pos_fn_param_as_bound<TSub,TSuper>(): (function<T as TSub super TSuper>(T):T) {
  throw new Exception();
}

function ret_pos_with_user_attr(): (function<<<__NoAutoBound>> T>(T):T) {
  throw new Exception();
}
