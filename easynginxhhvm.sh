#!/bin/bash

####################################
#Easy install of Nginx and HHVM    #
#on Ubuntu 10.04,12.04,14.04       #
####################################

#A script for automatic installation of Nginx with HHVM
#using prebuild packages on Ubuntu.Supports 10.04,12.04,14.04

DISTRO_VERSION=$(lsb_release -r) #find the release of your Ubuntu Distro

CHECK_RELEASE14_04="Release:" 
CHECK_RELEASE12_02="Release:"
CHECK_RELEASE10_04="Release:"

TAB=$'\t' #quote TAB character to TAB variable

VERSION14_04="14.04"
CHECK_RELEASE14_04+=${TAB} #add tab to CHECK_RELEASE
CHECK_RELEASE14_04+=${VERSION14_04} #add 14.04

VERSION12_04="12.04"
CHECK_RELEASE12_04+=${TAB} #add tab to CHECK_RELEASE
CHECK_RELEASE12_04+=${VERSION12_04} #add 12.04

VERSION10_04="10.04"
CHECK_RELEASE10_04+=${TAB} #add tab to CHECK_RELEASE
CHECK_RELEASE10_04+=${VERSION10_04} #add 10.04 



echo 
if [ "$DISTRO_VERSION" == "$CHECK_RELEASE14_04" ]
then
	echo 'Ubuntu 14.04 detected!'
	sudo apt-get update
	sudo apt-get install nginx #get nginx server 
	sudo service nginx stop #stop nginx for installing hhvm
	wget -O - http://dl.hhvm.com/conf/hhvm.gpg.key | sudo apt-key add - 
	echo deb http://dl.hhvm.com/ubuntu trusty main | sudo tee /etc/apt/sources.list.d/hhvm.list
	sudo apt-get update
	sudo apt-get install hhvm #install hhvm
	sudo service nginx restart #first restart nginx
	sudo service hhvm  restart #then hhvm
	echo 'Successfully installed HHVM and Nginx!'
elif [ "$DISTRO_VERSION" == "$CHECK_RELEASE12_04" ]
then
	echo 'Ubuntu 12.04 detected!'
	sudo apt-get update
	sudo apt-get install nginx #get nginx server 
	sudo service nginx stop #stop nginx for installing hhvm
	sudo add-apt-repository ppa:mapnik/boost
	wget -O - http://dl.hhvm.com/conf/hhvm.gpg.key | sudo apt-key add -
	echo deb http://dl.hhvm.com/ubuntu precise main | sudo tee /etc/apt/sources.list.d/hhvm.list
	sudo apt-get update
	sudo apt-get install hhvm #install hhvm
	sudo service nginx restart #first restart nginx
	sudo service hhvm  restart #then hhvm
	echo 'Successfully installed HHVM and Nginx!'

elif [ "$DISTRO_VERSION" == "$CHECK_RELEASE10_04" ]
then
	echo 'Ubuntu 10.04 detected!'
	sudo apt-get update
	sudo apt-get install nginx #get nginx server 
	sudo service nginx stop #stop nginx for installing hhvm
	sudo add-apt-repository ppa:ubuntu-toolchain-r/test
	wget -O - http://dl.hhvm.com/conf/hhvm.gpg.key | sudo apt-key add -
	echo deb http://dl.hhvm.com/ubuntu lucid main | sudo tee /etc/apt/sources.list.d/hhvm.list
	sudo apt-get update
	sudo apt-get install gcc-4.8 g++-4.8 gcc-4.8-base
	sudo apt-get install hhvm #install hhvm
	sudo service nginx restart #first restart nginx
	sudo service hhvm  restart #then hhvm
	echo 'Successfully installed HHVM and Nginx!'

else
	echo 'Your Distribution of Ubuntu is not supported'
fi
