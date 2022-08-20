@ECHO OFF

python3 build/fbcode_builder/getdeps.py build --src-dir=. watchman "--project-install-prefix=watchman:%userprofile%" --scratch-path "C:\open\scratch"

python3 build/fbcode_builder/getdeps.py fixup-dyn-deps --src-dir=. watchman  built "--project-install-prefix=watchman:%userprofile%" --final-install-prefix "%userprofile%" --scratch-path "C:\open\scratch"