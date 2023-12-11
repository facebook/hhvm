<?hh
<<__EntryPoint>> function main(): void {
echo "-- only two parameters --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/'),"\n";
echo "-- using empty array --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/', vec[]),"\n";
echo "-- using string instead of array --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/',''),"\n";
echo "-- using numeric keys masked as a string --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/',dict['1'=>'hello']),"\n";
echo "-- using string keys --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/',dict['test'=>'hello']),"\n";
echo "-- using same var in url and params --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/?test=hi',dict['test'=>'hello']),"\n";
echo "-- using null inside params --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/',dict['test'=>null]),"\n";
echo "-- putting oauth_signature inside by mistake --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/',dict['oauth_signature'=>'hello world']),"\n";
echo "-- merging url query and extra params --\n";
echo oauth_get_sbs('GET', 'http://127.0.0.1:12342/script?arg1=1',dict['arg2' => '2']),"\n";
}
