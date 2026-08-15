<?hh

class Yep {}
class Nope {}

function eek(?Nope $_):void {}

function hmm(shape(Nope...) $nopes): void {
   $x = Shapes::idx($nopes, 'x');
   eek($x);
}

function wut(shape(Yep...) $yeps): void {
  hmm($yeps);
}
