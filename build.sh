export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

make clean; bear -- make
#if [ $(make -j$(nproc) all &>/dev/null ; echo $?) -eq 0 ];
#then
#	gdb -x gdb
#else
#	echo "failed to build."
#fi

if [[ "$1" == "github" ]]; then
	make -j $(nproc) github 
else
	make -j$(nproc) all
fi
