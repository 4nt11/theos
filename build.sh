export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

make clean
#if [ $(make -j$(nproc) all &>/dev/null ; echo $?) -eq 0 ];
#then
#	gdb -x gdb
#else
#	echo "failed to build."
#fi

make -j$(nproc) all
