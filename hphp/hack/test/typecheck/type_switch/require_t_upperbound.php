<?hh

type MyDisableRuntimeTypecheck<T> = T;

interface IMyContra<-T> {}

function lambda_to_contra<T>((function(T): void) $_): IMyContra<T> {
  throw new Exception();
}

function mystrval(mixed $v): MyDisableRuntimeTypecheck<string> {
  throw new Exception();
}

function repro(): IMyContra<mixed> {
  return lambda_to_contra($target ==> {
    if ($target is KeyedContainer<_, _>) {
      $_ = 0;
    } else {
      mystrval($target);
    }
  });
}
