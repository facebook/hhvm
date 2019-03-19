<?php

const SESSION_FILE_PREFIX = "session_test_";
function open($save_path, $session_name) {

    ZendGoodExtSessionTestsSaveHandlerInc::$session_save_path = $save_path;
    ZendGoodExtSessionTestsSaveHandlerInc::$name = $session_name;
    echo "Open [${session_save_path},${session_name}]\n";
    return true;
}

function close() {

    echo "Close [".ZendGoodExtSessionTestsSaveHandlerInc::session_save_path.",${name}]\n";
    return true;
}

function read($id) {

    $session_save_path = ZendGoodExtSessionTestsSaveHandlerInc::$session_save_path;
    ZendGoodExtSessionTestsSaveHandlerInc::$session_id = $id;
    echo "Read [${session_save_path},${id}]\n";
    $session_file = $session_save_path."/".SESSION_FILE_PREFIX.$id;
    return (string) @file_get_contents($session_file);
}

function write($id, $session_data) {

    $session_save_path = ZendGoodExtSessionTestsSaveHandlerInc::$session_save_path;
    ZendGoodExtSessionTestsSaveHandlerInc::$session_id = $id;
    echo "Write [${session_save_path},${id},${session_data}]\n";
    $session_file = "$session_save_path/".SESSION_FILE_PREFIX.$id;
    if ($fp = fopen($session_file, "w")) {
        $return = fwrite($fp, $session_data);
        fclose($fp);
        return $return;
    }
    return false;
}

function destroy($id) {

    $session_save_path = ZendGoodExtSessionTestsSaveHandlerInc::$session_save_path;
    echo "Destroy [${session_save_path},${id}]\n";
    $session_file = "$session_save_path/".SESSION_FILE_PREFIX.$id;
    return unlink($session_file);
}

function gc($maxlifetime) {

    $directory = opendir(ZendGoodExtSessionTestsSaveHandlerInc::$session_save_path."/");
    $length = strlen(SESSION_FILE_PREFIX);
    while (($file = readdir($directory)) !== FALSE) {
        $qualified = (ZendGoodExtSessionTestsSaveHandlerInc::$session_save_path."/".$file);
        if (is_file($qualified) === TRUE) {
            if (substr($file, 0, $length) === SESSION_FILE_PREFIX) {
                unlink($qualified);
            }
        }
    }
    closedir($directory);
    return true;
}

abstract final class ZendGoodExtSessionTestsSaveHandlerInc {
  public static $session_save_path;
  public static $name;
  public static $session_id;
}
