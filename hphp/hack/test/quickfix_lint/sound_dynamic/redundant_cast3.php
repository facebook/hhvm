<?hh

<<__SupportDynamicType>>
function main1(float $f): void {
  (float) $f;
}

type Wrap<T> = T;

<<__SupportDynamicType>>
function main2(Wrap<float> $wf): void {
  (float) $wf;
}

<<__SupportDynamicType>>
function main2(vec<int> $_): void {
  (float) 42.0;
}
