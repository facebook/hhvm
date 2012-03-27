
#checkout-and-build.sh

git clone git://github.com/h4ck3rm1k3/hiphop-php.git 
mv hiphop-php hiphop-php-0.1
cd hiphop-php-0.1
git checkout latest-patch
dpkg-buildpackage -us -uc -j24