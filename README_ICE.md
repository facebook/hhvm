<h1>Hiphop Ice extension:</h1>

<h2>1.  The directory structure</h2>


<b>Idl file：</b>src/idl/ice.idl.php<br>
<b>Implement files：</b>src/runtime/ext/ext_ice.h<br>
		 src/runtime/ext/ext_ice.cpp<br>
<b>php interface file：</b>php/ice/Ice.php<br>
		    HHVM we custom the ice in the interface file, 
		    so the application HHVM call ice,
		    Can't quote the PHP file Ice.php，Need we defined this Ice.php file reference<br>
		    For Example:<br>
		    require “/export/hhvm/php/Ice.php<br>

<b>Test case:</b>test<br>

<h2>2.	Support functions：</h2>
Types of support:<br>
The basic types supported:<br>
String,Int,Long,Double<br>
Class does not support inheritance<br>
Exception class does not support inheritance<br>
Support Sequence<br>

Connection is a persistent connection object every thread.<br>


<h2>3.	Ice Support package installation：</h2>
version：3.4.1<br>
http://www.zeroc.com/download_3_4_1.html<br>


<h2>3.1.	Ice support package install</h2>
tar zxvf ThirdParty-Sources-3.4.1.tar.gz<br>
cd ThirdParty-Sources-3.4.1<br>
<h3>（1）	Mpp intall: </h3>
tar zxvf mcpp-2.7.2.tar.gz<br>
cd mcpp-2.7.2<br>
patch -p0 < ../mcpp/patch.mcpp.2.7.2<br>
./configure CFLAGS=-fPIC --enable-mcpplib --disable-shared  --prefix=/export/huzhiguang/files/mcpp-2.7.2<br>
make –j 16<br>
make install<br>
<h3>（2）	Berkeley DBinstall</h3>
Tar zxvf db-4.8.30.NC.tar.gz<br>
cd db-4.8.30.NC.tar.gz<br>
cd build_unix<br>
../dist/configure --enable-cxx --prefix= /export/huzhiguang/files/db-4.8.30.NC<br>
Make –j 16<br>
Make install<br>

Note:
If the system is 64 - bit Linux installed, you need to/export/huzhiguang/files/db - 4.8.30. NC lib change under the name lib64<br>
<h3>（3）	Install bzip2</h3>
tar zxvf bzip2-1.0.5.tar.gz<br>
cd bzip2-1.0.5<br>
make –j 16<br>
make install PREFIX=/export/huzhiguang/files/bzip2-1.0.5<br>
<h3>（4）	Install expat</h3>
tar zxvf expat-2.0.1.tar.gz<br>
cd expat-2.0.1<br>
./configure --prefix=/export/huzhiguang/files/expat-2.0.1/<br>
Make –j 16<br>
Make install<br>
<h3>（5）	Intall openssl</h3>
Tar zxvf openssl-0.9.8n.tar.gz<br>
Cd openssl-0.9.8n<br>
/config --prefix=/export/huzhiguang/files/openssl-0.9.8n<br>
Make –j 16<br>
Make install<br>


<h2>3.2.	Install ICE cpp</h2>
Tar zxvf Ice-3.4.1.tar.gz<br>
Cd Ice-3.4.1/cpp<br>
vi config/Make.rules<br>

Modify the prefix ice installation path and support package for reference path:<br>
=====================================================================================<br>
14： prefix          ?= /export/huzhiguang/files/Ice-$(VERSION)<br>
20：embedded_runpath_prefix ?= /export/huzhiguang/files/Ice-$(VERSION_MAJOR).$(VERSION_MINOR)<br>
66：BZIP2_HOME      ?= /export/huzhiguang/files/bzip2-1.0.5<br>
74：DB_HOME     ?= /export/huzhiguang/files/db-4.8.30.NC<br>
80：EXPAT_HOME      ?= /export/huzhiguang/files/expat-2.0.1<br>
87：OPENSSL_HOME        ?= /export/huzhiguang/files/openssl-0.9.8n<br>
93：MCPP_HOME       ?= /export/huzhiguang/files/mcpp-2.7.2<br>
=====================================================================================<br>
Modification has been completed, and then save<br>
Make –j 16<br>
Make install<br>

Configure the environment variables：<br>
vi ~/.bashrc<br>
Add the following content：<br>
=====================================================================================<br>
export ICE_HOME=/export/huzhiguang/files/Ice-3.4.1<br>
export PATH=$ICE_HOME:$PATH<br>
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ICE_HOME/lib64<br>
=====================================================================================<br>
excute :source ~/.bashrc<br>

Then enter the ICE_HOME/bin, check the LDD the executable file <br>
=====================================================================================<br>
[huzg@localhost bin]$ ldd slice2cpp<br>
        linux-vdso.so.1 =>  (0x00007ffff5554000)<br>
        libSlice.so.34 => /export/huzhiguang/files/Ice-3.4.1/lib64/libSlice.so.34 (0x00007fe0065fd000)<br>
        libIceUtil.so.34 => /export/huzhiguang/files/Ice-3.4.1/lib64/libIceUtil.so.34 (0x00007fe006382000)<br>
        libpthread.so.0 => /lib64/libpthread.so.0 (0x00007fe006152000)<br>
        librt.so.1 => /lib64/librt.so.1 (0x00007fe005f4a000)<br>
        libiconv.so.2 => /usr/local/lib/libiconv.so.2 (0x00007fe005c65000)<br>
        libstdc++.so.6 => /export/servers/gcc-4.6.1/lib64/libstdc++.so.6 (0x00007fe005961000)<br>
        libm.so.6 => /lib64/libm.so.6 (0x00007fe0056dd000)<br>
        libgcc_s.so.1 => /export/servers/gcc-4.6.1/lib64/libgcc_s.so.1 (0x00007fe0054c7000)<br>
        libc.so.6 => /lib64/libc.so.6 (0x00007fe005134000)<br>
        /lib64/ld-linux-x86-64.so.2 (0x00007fe006a48000)<br>
=====================================================================================<br>
Dynamic link library content is normal<br>

$ICE_HOME/lib64  so packages copy to $CMAKE_PREFIX_PATH/lib/Ice directory<br>
$ICE_HOME/include directory copy to $CMAKE_PREFIX_PATH/include/Ice directory<br>
cp -r $ICE_HOME/lib64   $CMAKE_PREFIX_PATH/lib/Ice <br>
cp -r $ICE_HOME/include $CMAKE_PREFIX_PATH/include/Ice<br>
<h3>4.	Hiphop Ice install way：</h3>
edit $HPHP_HOME/CMake/HPHPFind.cmake<br>
add content:<br>
    include_directories(/export/dev_hhvm/usr/include/Ice)<br>
     target_link_libraries(${target} /export/huzhiguang/files/Ice-3.4.1/lib64/libIce.so)<br>
     target_link_libraries(${target} /export/huzhiguang/files/Ice-3.4.1/lib64/libIceUtil.so)<br>
save this file<br>


cp ice.idl.php $HPHP_HOME/src/idl<br>
cd $HPHP_HOME/src<br>
cp ext_ice.h $HPHP_HOME/src/runtime/ext<br>
cp ext_ice.cpp $HPHP_HOME/src/runtime/ext<br>

Other ways in accordance with the HHVM compiler extensions can be added<br>

<h3>5.	Test case</h3>
wait for join<br>


