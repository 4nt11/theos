#include "status.h"
#include "kernel.h"
#include <stdint.h>

// since we don't yet have the concept of "exiting" the program (no processes), we'll call
// this function `kassert`. it won't die, but it will cry.
int kassert(int exit_code)
{
	if(!exit_code)
	{
		print("[!] error here");
		return exit_code;
	}
	return PEACHOS_ALLOK;
}
