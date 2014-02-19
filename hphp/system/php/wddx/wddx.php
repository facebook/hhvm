<?php
class wddx{
  private $packetId;
  
  private static $map;
  private static $dtdMap;
  
  private static function init(){  
    if(isset($map)&&isset($dtdMap)){ return ;}
    self::$map = [
      "binary"  => [false,["<binary>","</binary>"]],
      "boolean"  => [false,["<boolean value='","'/>"]],
      "char"    => [false,["<char code='","'/>"]],
      "null"    => [false,["<null/>",""]],
      "integer"  => [false,["<number>","</number>"]],  
      "double"  => [false,["<number>","</number>"]],
      "string"  => [false,["<string>","</string>"]],  
      "array"    => [true,["<array length='%d'>","</array>"]],
      "struct"  => [true,["<struct>","</struct>"]],
      "var"    => [true,["<var name='%s'>","</var>"]],    
      "object"  => [true,["<struct>","</struct>"]],
    ];
    self::$dtdMap = [  
      "comment"  => ["<comment>","</comment>"],
      "data"    => ["<data>","</data>"],
      "header"  => ["<header>","</header>"],
      "noheader"  => ["<header/>",""],
      "packet"  => ["<wddxPacket version='1.0'>","</wddxPacket>"],
    ];
  }

  private static function is_assoc($array) {
    return (bool)count(array_filter(array_keys($array), 'is_string'));
  }
  
  private static function serializeVar($name , $value){
    $tags = self::$map["var"][1];
    $tags[0] = sprintf($tags[0],$name);
    return $tags[0] . self::serialize($value) . $tags[1];
  }
  
  private static function serialize($var){
    $type = gettype($var);
    $typeArray = self::$map[$type];
    $container = $typeArray[0];
    $tags =  $typeArray[1];
        
    if(!$container){
      if($type == "boolean"){
        $var = ($var) ? 'true' : 'false';
      }  
      $result = $tags[0] . $var . $tags[1];  
    }else{      
      if($type == "array"){
        $tags[0] = sprintf($tags[0] , count($var));
        if(self:: is_assoc($var)){
          $tags = self::$map["struct"][1];
          foreach($var as $varName => $varValue){
            $inner .= self::serializeVar($varName,$varValue);
          }    
        }else{
          foreach($var as $element){
            $inner .= self::serialize($element);
          }        
        }
      }
      if($type == "object"){
        $className = get_class($var);
        $varArray = get_object_vars($var);
        $inner .= self::serializeVar("php_class_name",$className);
        foreach($varArray as $varName => $varValue){
          $inner .= self::serializeVar($varName,$varValue);
        }        
      }
      $result = $tags[0] . $inner . $tags[1];
    }  
    return $result;
  }
  
  public static function wddx_serialize_value(){
    self::init();  
    $result .= self::$dtdMap["packet"][0];
    $numargs = func_num_args();
    if($numargs>2 || $numargs<0){
      return false;
    }else{
      $vars = func_get_args()[0];
    }
    if($numargs==1){
      $result .= self::$dtdMap["noheader"][0];
    }else{    
      $comment = func_get_args()[1];
      $result .=self::$dtdMap["header"][0].
                self::$dtdMap["comment"][0].
                $comment.
                self::$dtdMap["comment"][1].
                self::$dtdMap["header"][1];
    }    
    $result .=self::$dtdMap["data"][0].
              self::serialize($vars).
              self::$dtdMap["data"][1].
              self::$dtdMap["packet"][1];  
    return $result;
  }
}

function wddx_serialize_value(){
  $instance = new wddx();
    return call_user_func_array(array($instance,'wddx_serialize_value')
           , func_get_args());
}
