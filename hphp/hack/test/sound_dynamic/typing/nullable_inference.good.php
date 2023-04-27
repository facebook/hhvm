<?hh

<<__SupportDynamicType>>
function my_map_nullable<Tx as supportdyn<mixed> , Ty as supportdyn<mixed> >(?Tx $x, supportdyn<(function(Tx): ~Ty)> $fn
): void {
}

<<__SupportDynamicType>>
interface Getter<T as supportdyn<mixed> > {
  public function getPlaceholder(): ~?T;
}

<<__SupportDynamicType>>
function onPlaceholder<T as supportdyn<mixed> >(Getter<T> $field, supportdyn<(function(T): ~bool)> $callback): void {
    my_map_nullable($field->getPlaceholder(), $callback);
}
