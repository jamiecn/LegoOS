/*
 * Copyright (c) 2016 Wuklab, Purdue University. All rights reserved.
 */

#ifndef _X86_BOOT_BOOT_H_
#define _X86_BOOT_BOOT_H_

#define STACK_SIZE	512

/* Useful macros */
#define BUILD_BUG_ON(condition)	((void)sizeof(char[1 - 2*!!(condition)]))

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof(*(x)))

#ifndef __ASSEMBLY__

#include <stdarg.h>
#include <disos/types.h>

extern struct setup_header hdr;
extern struct boot_params boot_params;

static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

static inline int isxdigit(int ch)
{
	if (isdigit(ch))
		return true;

	if ((ch >= 'a') && (ch <= 'f'))
		return true;

	return (ch >= 'A') && (ch <= 'F');
}

#define cpu_relax()	asm volatile("rep; nop")

/* Basic port I/O */
static inline void outb(u8 v, u16 port)
{
	asm volatile("outb %0,%1" : : "a" (v), "dN" (port));
}
static inline u8 inb(u16 port)
{
	u8 v;
	asm volatile("inb %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void outw(u16 v, u16 port)
{
	asm volatile("outw %0,%1" : : "a" (v), "dN" (port));
}
static inline u16 inw(u16 port)
{
	u16 v;
	asm volatile("inw %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void outl(u32 v, u16 port)
{
	asm volatile("outl %0,%1" : : "a" (v), "dN" (port));
}
static inline u32 inl(u16 port)
{
	u32 v;
	asm volatile("inl %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void io_delay(void)
{
	const u16 DELAY_PORT = 0x80;
	asm volatile("outb %%al,%0" : : "dN" (DELAY_PORT));
}

/* These functions are used to reference data in other segments. */

static inline u16 cs(void)
{
	u16 seg;
	asm("movw %%cs,%0" : "=rm" (seg));
	return seg;
}

static inline u16 es(void)
{
	u16 seg;
	asm("movw %%es,%0" : "=rm" (seg));
	return seg;
}

static inline u16 ds(void)
{
	u16 seg;
	asm("movw %%ds,%0" : "=rm" (seg));
	return seg;
}

static inline void set_fs(u16 seg)
{
	asm volatile("movw %0,%%fs" : : "rm" (seg));
}
static inline u16 fs(void)
{
	u16 seg;
	asm volatile("movw %%fs,%0" : "=rm" (seg));
	return seg;
}

static inline void set_gs(u16 seg)
{
	asm volatile("movw %0,%%gs" : : "rm" (seg));
}
static inline u16 gs(void)
{
	u16 seg;
	asm volatile("movw %%gs,%0" : "=rm" (seg));
	return seg;
}

typedef unsigned int addr_t;

static inline u8 rdfs8(addr_t addr)
{
	u8 v;
	asm volatile("movb %%fs:%1,%0" : "=q" (v) : "m" (*(u8 *)addr));
	return v;
}
static inline u16 rdfs16(addr_t addr)
{
	u16 v;
	asm volatile("movw %%fs:%1,%0" : "=r" (v) : "m" (*(u16 *)addr));
	return v;
}
static inline u32 rdfs32(addr_t addr)
{
	u32 v;
	asm volatile("movl %%fs:%1,%0" : "=r" (v) : "m" (*(u32 *)addr));
	return v;
}

static inline void wrfs8(u8 v, addr_t addr)
{
	asm volatile("movb %1,%%fs:%0" : "+m" (*(u8 *)addr) : "qi" (v));
}
static inline void wrfs16(u16 v, addr_t addr)
{
	asm volatile("movw %1,%%fs:%0" : "+m" (*(u16 *)addr) : "ri" (v));
}
static inline void wrfs32(u32 v, addr_t addr)
{
	asm volatile("movl %1,%%fs:%0" : "+m" (*(u32 *)addr) : "ri" (v));
}

static inline u8 rdgs8(addr_t addr)
{
	u8 v;
	asm volatile("movb %%gs:%1,%0" : "=q" (v) : "m" (*(u8 *)addr));
	return v;
}
static inline u16 rdgs16(addr_t addr)
{
	u16 v;
	asm volatile("movw %%gs:%1,%0" : "=r" (v) : "m" (*(u16 *)addr));
	return v;
}
static inline u32 rdgs32(addr_t addr)
{
	u32 v;
	asm volatile("movl %%gs:%1,%0" : "=r" (v) : "m" (*(u32 *)addr));
	return v;
}

static inline void wrgs8(u8 v, addr_t addr)
{
	asm volatile("movb %1,%%gs:%0" : "+m" (*(u8 *)addr) : "qi" (v));
}
static inline void wrgs16(u16 v, addr_t addr)
{
	asm volatile("movw %1,%%gs:%0" : "+m" (*(u16 *)addr) : "ri" (v));
}
static inline void wrgs32(u32 v, addr_t addr)
{
	asm volatile("movl %1,%%gs:%0" : "+m" (*(u32 *)addr) : "ri" (v));
}

/* Note: these only return true/false, not a signed return value! */
static inline int memcmp_fs(const void *s1, addr_t s2, size_t len)
{
	u8 diff;
	asm volatile("fs; repe; cmpsb; setnz %0"
		     : "=qm" (diff), "+D" (s1), "+S" (s2), "+c" (len));
	return diff;
}
static inline int memcmp_gs(const void *s1, addr_t s2, size_t len)
{
	u8 diff;
	asm volatile("gs; repe; cmpsb; setnz %0"
		     : "=qm" (diff), "+D" (s1), "+S" (s2), "+c" (len));
	return diff;
}

struct biosregs {
	union {
		struct {
			u32 edi;
			u32 esi;
			u32 ebp;
			u32 _esp;
			u32 ebx;
			u32 edx;
			u32 ecx;
			u32 eax;
			u32 _fsgs;
			u32 _dses;
			u32 eflags;
		};
		struct {
			u16 di, hdi;
			u16 si, hsi;
			u16 bp, hbp;
			u16 _sp, _hsp;
			u16 bx, hbx;
			u16 dx, hdx;
			u16 cx, hcx;
			u16 ax, hax;
			u16 gs, fs;
			u16 es, ds;
			u16 flags, hflags;
		};
		struct {
			u8 dil, dih, edi2, edi3;
			u8 sil, sih, esi2, esi3;
			u8 bpl, bph, ebp2, ebp3;
			u8 _spl, _sph, _esp2, _esp3;
			u8 bl, bh, ebx2, ebx3;
			u8 dl, dh, edx2, edx3;
			u8 cl, ch, ecx2, ecx3;
			u8 al, ah, eax2, eax3;
		};
	};
};

/* header.S */
void __attribute__((noreturn)) die(void);

/* bioscall.S */
void intcall(unsigned char int_no, const struct biosregs *ireg, struct biosregs *oreg)
__attribute__ ((regparm(3)));

/* regs.c */
void initregs(struct biosregs *regs);

/* printf.c */
int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int printf(const char *fmt, ...);

/* tty.c */
void puts(const char *);
void putchar(int);
int getchar(void);
void kbd_flush(void);
int getchar_timeout(void);

/* string.c */
int strcmp(const char *str1, const char *str2);
int strncmp(const char *cs, const char *ct, size_t count);
size_t strnlen(const char *s, size_t maxlen);
unsigned int atou(const char *s);
unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base);
size_t strlen(const char *s);

extern const char kernel_version[];

#endif

#endif /* _X86_BOOT_BOOT_H_ */
