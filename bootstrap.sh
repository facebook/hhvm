# call this on a new instance of a machine to get things rolling

apt-get update apt-get install gcc apt-get install build-essential git

git clone git://github.com/h4ck3rm1k3/hiphop-php.git 
mv hiphop-php hiphop-php-0.1
cd hiphop-php-0.1
git checkout latest-patch

apt-get install debhelper cmake libtbb-dev libmcrypt-dev re2c binutils-dev libonig-dev libmysqlclient15-dev libgd2-xpm-dev libmemcached-dev libboost-all-dev libpcre3-dev libevent-dev libboost-all-dev libxml2-dev libbz2-dev libncurses-dev libreadline-dev libc-client2007e-dev libcap-dev  autoconf automake autotools-dev bison curl flex libapache2-mod-php5 libapr1 libaprutil1 libaprutil1-dbd-sqlite3 libaprutil1-ldap libcurl4-openssl-dev libltdl-dev libltdl7 libmhash2 libqdbm14 libssh2-1-dev libtool m4 mcrypt php5-cli php5-common php5-dev php5-suhosin shtool ssl-cert emacs23-nox

dpkg-buildpackage -us -uc -j 24