<?php

function offsetGet($index) {
  echo ("GET0: $index\n");
}
class ArrayAccessImpl implements ArrayAccess {
  public $data = array();
  public function offsetUnset($index) {
 echo "UNSET: $index\n";
 }
  public function offsetSet($index, $value) {
    echo ("SET: $index\n");
    if(isset($data[$index])) {
        unset($data[$index]);
    }
    $u = &$this->data[$index];
    if(is_array($value)) {
        $u = new ArrayAccessImpl();
        foreach($value as $idx=>$e)            $u[$idx]=$e;
    }
 else        $u=$value;
  }
  public function offsetGet($index) {
    echo ("GET: $index\n");
    if(!isset($this->data[$index]))        $this->data[$index]=new ArrayAccessImpl();
    return $this->data[$index];
  }
  public function offsetExists($index) {
    echo ("EXISTS: $index\n");
    if(isset($this->data[$index])) {
        if($this->data[$index] instanceof ArrayAccessImpl) {
            if(count($this->data[$index]->data)>0)                return true;
            else                return false;
        }
 else            return true;
    }
 else        return false;
  }
}
class ArrayAccessImpl2 extends ArrayAccessImpl {
  public function offsetUnset($index) {
 echo "UNSET2: $index\n";
 }
  public function offsetSet($index, $value) {
    echo ("SET2: $index\n");
    if(isset($data[$index])) {
        unset($data[$index]);
    }
    $u = &$this->data[$index];
    if(is_array($value)) {
        $u = new ArrayAccessImpl();
        foreach($value as $idx=>$e)            $u[$idx]=$e;
    }
 else        $u=$value;
  }
  public function offsetGet($index) {
    echo ("GET2: $index\n");
    if(!isset($this->data[$index]))        $this->data[$index]=new ArrayAccessImpl();
    return $this->data[$index];
  }
  public function offsetExists($index) {
    echo ("EXISTS2: $index\n");
    if(isset($this->data[$index])) {
        if($this->data[$index] instanceof ArrayAccessImpl) {
            if(count($this->data[$index]->data)>0)                return true;
            else                return false;
        }
 else            return true;
    }
 else        return false;
  }
}
offsetGet('foo');
$data = new ArrayAccessImpl();
$data['string']="Just a simple string";
$data['number']=33;
$data['array']['another_string']="Alpha";
$data['array']['some_object']=new stdClass();
$data['array']['another_array']['x']['y']="Beta";
$data['blank_array']=array();
print_r(isset($data['array']));
print_r($data['array']['non_existent']);
print_r(isset($data['array']['non_existent']));
print_r($data['blank_array']);
print_r(isset($data['blank_array']));
unset($data['blank_array']);
print_r($data);
$data2 = new ArrayAccessImpl2();
$data2['string']="Just a simple string";
$data2['number']=33;
$data2['array']['another_string']="Alpha";
$data2['array']['some_object']=new stdClass();
$data2['array']['another_array']['x']['y']="Beta";
$data2['blank_array']=array();
print_r(isset($data2['array']));
print_r($data2['array']['non_existent']);
print_r(isset($data2['array']['non_existent']));
print_r($data2['blank_array']);
print_r(isset($data2['blank_array']));
unset($data2['blank_array']);
print_r($data2);
