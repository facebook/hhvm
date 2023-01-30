#!/bin/bash

if [ -f /etc/init.d/hhvm ]
then
	/etc/init.d/hhvm start
fi

#!/bin/bash

apache_check_installed() {
	echo "Checking if Apache is installed"
	if [ \( ! -d /etc/apache2/mods-enabled \) -o \( ! -d /etc/apache2/mods-available \) ]
	then
		echo "WARNING: Couldn't find Apache2 configuration paths, not configuring"
		return 1
	fi
	echo "Detected Apache installation"
	return 0
}

apache_check_custom() {
	echo "Looking for custom proxy configuration"
	if [ ! -z "$(grep 9000 -R /etc/apache2/mods-enabled | grep ProxyPass)" ]
	then
		echo "WARNING: Clashing existing configuration detected, not configuring"
		return 0
	fi
	echo "No custom proxy configuration found"
	return 1
}

apache_check_module() {
	echo "Checking for ${2} ${1}.${3}"
	if [ ! -f "/etc/apache2/mods-${2}/${1}.${3}" ]
	then
		echo "Not found"
		return 1
	fi
	echo "Found, checking for loading directives"
	if [ "$3" == "load" ]
	then
		result=$(grep -E "^[^#]*LoadModule\\s*${1}_module" "/etc/apache2/mods-${2}/${1}.${3}")
 		if [ -z "$result" ]
		then
			"Loading directive not found"
			return 1
		fi
	fi
	echo "Detected ${2} ${1}.${3} configuration, setting up integration"
	return 0
}

apache_force_enable_module() {
	echo "Force enabling module ${1}.${2}"
	if [ ! -f "/etc/apache2/mods-available/${1}.${2}" ]
	then
		echo "WARNING: Unsupported ${1}, not configuring"
		return 1
	fi
	echo "Available module found"
	rm -f "/etc/apache2/mods-enabled/${1}.${2}"
	echo "Removed possible duplicates"
	apache_enable_module $1 $2
	echo "Completed force enabling"
	return 0
}

apache_enable_module() {
	echo "Enabling module ${1}.${2}"
	if [ ! -f "/etc/apache2/mods-available/${1}.${2}" ]
	then
		echo "WARNING: Could not enable ${1}, not configuring"
		return 1
	fi
	echo "Found available module"
	if [ -f "/etc/apache2/mods-enabled/${1}.${2}" ]
	then
		echo "Module already enabled"
		return 0
	fi
	echo "Creating a symlink"
	ln -s "../mods-available/${1}.${2}" "/etc/apache2/mods-enabled/${1}.${2}"
	echo "Finished creating a symlink"
	return 0
}

apache_disable_module() {
	echo "Disabling a module ${1}.${2}"
	rm -f "/etc/apache2/mods-enabled/${1}.${2}"
	echo "Finished disabling a module"
}

apache_restart() {
	echo "Restarting apache"
	if [ ! -f "/etc/init.d/apache2" ]
	then
		echo "Apache init.d script not detected"
		return 0
	fi
	result=$(/etc/init.d/apache2 status | grep 'is running')
	if [ ! -z "$result" ]
	then
		echo "Apache is running, restarting"
		/etc/init.d/apache2 restart
		echo "Finished restarting"
	fi
	echo "Finished restarting apache"
	return 0
}

#!/bin/bash

apache_postinst() {
	if ! apache_check_installed
	then
		return 0
	fi

	if apache_check_custom
	then
		return 0
	fi

	names="proxy_fcgi fastcgi fcgid"

	for name in $names
	do
		if ! apache_check_module "${name}" "enabled" "load"; then continue; fi
		if [ "$name" == "proxy_fcgi" ]
		then
			if ! apache_check_module "proxy" "enabled" "load"; then continue; fi
			if ! apache_check_module "proxy" "enabled" "conf"; then continue; fi
		fi
		apache_force_enable_module "hhvm_${name}" "conf"
		apache_restart
		return 0
	done

	for name in $names
	do
		if ! apache_check_module "${name}" "available" "load"; then continue; fi
		if ! apache_check_module "hhvm_${name}" "available" "conf"; then continue; fi
		if [ "$name" == "proxy_fcgi" ]
		then
			if ! apache_check_module "proxy" "available" "load"; then continue; fi
			if ! apache_check_module "proxy" "available" "conf"; then continue; fi
		fi
		if [ "$name" == "proxy_fcgi" ]
		then
			apache_enable_module "proxy" "load"
			apache_enable_module "proxy" "conf"
		fi
		apache_enable_module "$name" "load"
		apache_force_enable_module "hhvm_${name}" "conf"
		apache_restart
		return 0
	done

	echo "No matching Apache configuration found, not configuring"
	return 0
}

apache_postinst

#!/bin/bash

insert_line() {
	file=`mktemp`
	sed -E "s/^(\\s*)${2}$/&\\n\\1${3}/" $1 > $file
	cat $file > $1
}

remove_line() {
	file=`mktemp`
	sed -E "/^.*${2}.*$/d" $1 > $file
	cat $file > $1
}

check_line() {
	if [ -z "$(grep $2 $1)" ]
	then
		return 0
	fi
	return 1
}

nginx_check_installed() {
	echo "Checking if Nginx is installed"
	if [ -d "/etc/nginx/conf.d/" ]
	then
		echo "Detected Nginx installation"
		return 0
	fi
	echo "Nginx not found"
	return 1
}

nginx_check_custom() {
	echo "Checking for custom Nginx configuration"
	if [ ! \( -z "$(grep -E "^[^#]*(fastcgi|hhvm.conf)" "/etc/nginx/nginx.conf")" \) \
	  -o ! \( -z "$(grep -ER "^[^#]*(fastcgi|hhvm.conf)" "/etc/nginx/conf.d/")" \) \
          -o ! \( -z "$(grep -ER "^[^#]*(fastcgi|hhvm.conf)" "/etc/nginx/sites-enabled/")" \) ]
	then
		echo "WARNING: Detected clashing configuration. Look at /etc/nginx/hhvm.conf for information how to connect to the hhvm fastcgi instance."
		return 0
	fi
	return 1
}

nginx_enable_module() {
	echo "Enabling hhvm Nginx module"
	insert_line "/etc/nginx/sites-enabled/default" 'server_name.*$' 'include hhvm.conf;'
	echo "Finished enabling module"
}

nginx_disable_module() {
	echo "Disabling hhvm Nginx module"
	remove_line "/etc/nginx/sites-enabled/default" "hhvm.conf"
	echo "Finished disabling module"
}

nginx_restart() {
	echo "Restarting Nginx"
	if [ ! -f "/etc/init.d/nginx" ]
	then
		echo "Nginx init.d script not found"
		return 0
	fi
	result=$(/etc/init.d/nginx status | grep 'is running')
	if [ ! -z "$result" ]
	then
		echo "Nginx is running, restarting"
		/etc/init.d/nginx stop
		/etc/init.d/nginx start
		echo "Restarted nginx"
	fi
	echo "Finished restarting Nginx"
}

#!/bin/bash

nginx_postinst() {
	if ! nginx_check_installed
	then
		return 0
	fi

	if nginx_check_custom
	then
		return 0
	fi

	nginx_enable_module
	nginx_restart
	return 0
}

nginx_postinst
