<?hh


<<__EntryPoint>>
function main_preg_match_stack() :mixed{
$str = <<<'EOD'
<div class="operations_half_width clearfloat"><form id="copy_db_form" class="ajax" method="post" action="db_operations.php"onsubmit="return emptyFormElements(this, 'newname')"><input type="hidden" name="db_collation" value="db1" />
<input type="hidden" name="db_copy" value="true" />
<input type="hidden" name="db" value="pma" /><input type="hidden" name="lang" value="en" /><input type="hidden" name="token" value="token" /><fieldset><legend><img src="themes/dot.gif" title="" alt="" class="icon ic_b_edit" />Copy database to:</legend><input type="text" name="newname" size="30" class="textfield" value="" required="required" /><br /><input type="radio" name="what" id="what_structure" value="structure" />
<label for="what_structure">Structure only</label><br />
<input type="radio" name="what" id="what_data" value="data" checked="checked" />
<label for="what_data">Structure and data</label><br />
<input type="radio" name="what" id="what_dataonly" value="dataonly" />
<label for="what_dataonly">Data only</label><br />
<input type="checkbox" name="create_database_before_copying" value="1" id="checkbox_create_database_before_copying"checked="checked" /><label for="checkbox_create_database_before_copying">CREATE DATABASE before copying</label><br /><input type="checkbox" name="drop_if_exists" value="true"id="checkbox_drop" /><label for="checkbox_drop">Add DROP TABLE / DROP VIEW</label><br /><input type="checkbox" name="sql_auto_increment" value="1" checked="checked" id="checkbox_auto_increment" /><label for="checkbox_auto_increment">Add AUTO_INCREMENT value</label><br /><input type="checkbox" name="add_constraints" value="1"id="checkbox_constraints" /><label for="checkbox_constraints">Add constraints</label><br /><input type="checkbox" name="switch_to_new" value="true"id="checkbox_switch"/><label for="checkbox_switch">Switch to copied database</label></fieldset><fieldset class="tblFooters"><input type="submit" name="submit_copy" value="Go" /></fieldset></form></div>
EOD;

$pattern = "/.*db_operations.php(.|[\\n])*db_copy([\\n]|.)*Copy database to.*/m";

var_dump(preg_match($pattern, $str));
}
