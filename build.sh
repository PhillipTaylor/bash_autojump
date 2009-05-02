
if [ -d "bash-4.0" ]; then
	echo "Deleting existing bash-4.0 folder"
	rm -rf "bash-4.0"
fi

if [ ! -f "bash-4.0.tar.gz" ]; then
	echo "This repo only contains the bash-4.0 patch."
	echo "It needs the bash 4 source code before a new version of bash can be built."
	echo "Permission to download this from gnu.org using the following command:"
	echo "wget http://ftp.gnu.org/pub/gnu/bash/bash-4.0.tar.gz"
	echo -n "Is this okay? [Y/n]: "
	read PERMISSION

	if [ "$PERMISSION" == "Y" ] || [ "$PERMISSION" == "y" ]; then
		wget "http://ftp.gnu.org/pub/gnu/bash/bash-4.0.tar.gz"

		if [ $? -ne 0 ]; then
			echo "Download failed :-( Setup cannot continue"
			exit
		fi
	else
		echo "user cancelled. No BASH source to build against."
		exit
	fi
fi

echo "untaring new bash-4.0 folder"
tar -xf "bash-4.0.tar.gz"

`bash-4.0/configure --help`
echo -n "Enter ./configure options for bash: (default=blank): "
read CONFIG_OPTIONS

echo "create the symlinks into the bash project"
pushd bash-4.0/builtins
ln -s ../../ajump.def ajump.def
ln -s ../../autojump.h autojump.h
ln -s ../../autojump.c autojump.c
popd

echo "patching builtins/common.c"
patch bash-4.0/builtins/common.c common.c.patch

echo "patching builtins/Makefile.in"
patch bash-4.0/builtins/Makefile.in Makefile.in.patch

pushd "bash-4.0"

./configure "$CONFIG_OPTIONS"
make

popd

echo "To install autojump, go into the bash-4.0 folder and type 'make install'"
echo "script finished"
