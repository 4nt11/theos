# theos
*theos* is a kernel project made with the intention of understanding how operating systems work. a secondary purpose is to serve as an education tool for my future students.

## docs
all the documentation is in the `docs/` directory.

## building
to build the project, you will need a cross compiler. if you don't know how to get one, please read [this](https://wiki.osdev.org/GCC_Cross-Compiler#Preparing_for_the_build). after that, be sure to change the environment variables found in the `build.sh` script to point to your binaries.

```
git clone https://github.com/4nt11/theos
cd theos
chmod +x build.sh
vim build.sh # make your changes!
./build.sh
```

## built binaries
i also leave some built binaries in the `bin/` directory. use them if you don't want to get your own cross-compiller. i dont recommend it, since building the cross-compilling tools are part of the learning experience, but hey, you do you.

## running
you can run the kernel using two main approaches.

### gdb script
if you want a debugging session, you can use the GDB script provided. it sets up all the symbols needed for debugging and step-by-step execution.

```
gdb -x gdb_script
```

### qemu
if you want to just launch the kernel ~(there's not much to do in there for now)~, you can use `qemu-system-i386`.

```
qemu-system-i386 -hda build/os.bin
```

## how to contact me
if you have any doubts about the code or you don't understand a concept, feel free to contact me.
- [linkedin](https://www.linkedin.com/in/4nt1/)
- [email](mailto:4nt1@resacachile.cl)
- [telegram](https://t.me/clp_c)
- [mastodon](https://lile.cl/@4nt1)

## AI disclaimer
although I do use AI, not a single line of code was written by an AI. I use AI mostly for explainations of things that I can't quite get the hang of or to help me document the code.

## credits
this OS is made possible thanks to the amazing content made by the people at Dragonzap. thx for the course :)
