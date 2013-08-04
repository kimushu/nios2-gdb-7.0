
__attribute__((naked)) int write(int file, char *ptr, int len)
{
	__asm__(
	"movi r2, -1\n\t"
	"ori r0, r0, 0xcafe\n\t"
	"ori r0, r0, 0x0001\n\t"
	"ret"
	);
}

__attribute__((naked)) void _exit(int exitcode)
{
	__asm__(
	"ori r0, r0, 0xcafe\n\t"
	"ori r0, r0, 0x0010\n\t"
	);
	while(1);
}

