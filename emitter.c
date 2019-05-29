#define Assert(x) \
if (!(x)) { MessageBoxA(0, #x, "Assertion failed!", MB_OK); __debugbreak(); }

typedef enum RegisterKind
{
	RAX = 0,
	RCX = 1,
	RDX = 2,
	RBX = 3,
	RSP = 4,
	RBP = 5,
	RSI = 6,
	RDI = 7,
	R8 = 8,
	R9 = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15,
	RIP = 16,
} RegisterKind;

enum
{
	X1 = 0,
	X2 = 1,
	X4 = 2,
	X8 = 3,
};

enum
{
	DIRECT = 3,
	INDIRECT = 0,
	INDIRECT_DISP8 = 1,
	INDIRECT_DISP32 = 2,
};

enum
{
	MAX_CODE = 32768,
};

uint8_t code[MAX_CODE];
uint8_t *emit_ptr = code;

void emit(uint8_t byte)
{
	*emit_ptr = byte;
	emit_ptr++;
}

void emit32(uint32_t byte4)
{
	emit(byte4 & 0xFF);
	emit((byte4 >> 8) & 0xFF);
	emit((byte4 >> 16) & 0xFF);
	emit((byte4 >> 24) & 0xFF);
}

void emit_mod_rx_rm(uint8_t mod, uint8_t rx, uint8_t rm)
{
	Assert(mod < 4);
	Assert(rx < 16);
	Assert(rm < 16);
	emit((mod << 6) | ((rx & 7) << 3) | (rm & 7));
}

// mov rax, qword ptr[rcx + rdx * 4 + 12h]
// 48 8B 44 91 12       mov         rax, qword ptr[rcx + rdx * 4 + 12h]

void emit_rex(uint8_t rx, uint8_t base)
{
	emit(0x48 | (base >> 3) | ((rx >> 3) << 2));
}

void emit_rex_indexed(uint8_t rx, uint8_t base, uint8_t index)
{
	emit(0x48 | (base >> 3) | ((index >> 3) << 1) | ((rx >> 3) << 2));
}

// if inderect, 0-bit disp = mod 0;
// if inderect, 8-bit disp = mod 1;
// if inderect, 32-bit disp = mod 2;
// if direct, base reg goes in rm;
// unless base reg is rsp, then need a sib encoding with base = 4, index = 4, scale = any
// if direct = mod 3;

// TODO: refactor

#define emit_r_r(op, dst, src) \
	emit_rex(dst, src); \
	emit_##op##_r(); \
	emit_mod_rx_rm(DIRECT, dst, src); \

#define emit_r_m(op, dst, src) \
	emit_rex(dst, src); \
	emit_##op##_r(); \
	emit_mod_rx_rm(INDIRECT, dst, src); \

#define emit_r_md(op, dst, src, disp) \
	emit_rex(dst, src); \
	emit_##op##_r(); \
	emit_mod_rx_rm(INDIRECT_DISP32, dst, src); \
	emit32(disp); \

#define emit_r_i(op, dst, immediate) \
	emit_rex(0, dst); \
	emit_##op##_i(); \
	emit_mod_rx_rm(DIRECT, 0, dst); \
	emit32(immediate); \

#define emit_m_r(op, dst, src) \
	emit_rex(src, dst); \
	emit_##op##_rm(); \
	emit_mod_rx_rm(INDIRECT, src, dst); \

#define emit_m_i(op, dst, immediate) \
	emit_rex(0, dst); \
	emit_##op##_i(); \
	emit_mod_rx_rm(INDIRECT, 0, dst); \
	emit32(immediate); \

#define emit_md_i(op, rm, rm_disp, immediate) \
	emit_rex(0, rm); \
	emit_##op##_i(); \
	emit_mod_rx_rm(INDIRECT_DISP32, 0, rm); \
	emit32(rm_disp); \
	emit32(immediate); \

#define emit_sib_r(op, dst_base, scale, dst_index, src) \
	emit_rex_indexed(src, dst_base, dst_index); \
	emit_##op##_rm(); \
	emit_mod_rx_rm(INDIRECT, src, RSP); \
	emit_mod_rx_rm(scale, dst_index, dst_base); \

#define emit_sib_i(op, rm, scale, index, immediate) \
	emit_rex_indexed(0, rm, index); \
	emit_##op##_i(); \
	emit_mod_rx_rm(INDIRECT, 0, RSP); \
	emit_mod_rx_rm(scale, index, rm); \
	emit32(immediate); \

#define emit_sibd_i(op, rm, scale, index, disp, immediate) \
	emit_rex_indexed(0, rm, index); \
	emit_##op##_i(); \
	emit_mod_rx_rm(INDIRECT_DISP32, 0, RSP); \
	emit_mod_rx_rm(scale, index, rm); \
	emit32(disp); \
	emit32(immediate); \

#define emit_r_sib(op, rx, scale, rm, index) \
	emit_rex_indexed(rx, rm, index); \
	emit_##op##_r(); \
	emit_mod_rx_rm(INDIRECT, rm, RSP); \
	emit_mod_rx_rm(scale, index, rm); \

#define emit_r_sibd(op, rx, scale, rm, index, disp) \
	emit_rex_indexed(rx, rm, index); \
	emit_##op##_r(); \
	emit_mod_rx_rm(INDIRECT_DISP32, rx, RSP); \
	emit_mod_rx_rm(scale, index, rm); \
	emit32(disp); \

#define emit_x_r(op, source) \
	emit_rex(0, source); \
	emit_##op##_x(); \
	emit_mod_rx_rm(DIRECT, extension_##op##_x, source); \

#define emit_x_m(op, source) \
	emit_rex(0, source); \
	emit_##op##_x(); \
	emit_mod_rx_rm(INDIRECT, extension_##op##_x, source); \

#define emit_x_d(op, source_disp) \
	emit_rex(0, 0); \
	emit_##op##_x(); \
	emit_mod_rx_rm(INDIRECT, extension_##op##_x, RSP); \
	emit_mod_rx_rm(X1, RSP, RBP); \
	emit32(source_disp); \

#define emit_x_md(op, dst, src, disp) \
	emit_rex(0, src); \
	emit_##op##_x(); \
	emit_mod_rx_rm(INDIRECT_DISP32, extension_##op##_x, src); \
	emit32(disp); \

#define emit_i(op, immediate) \
	emit_rex(0, 0); \
	emit_##op##_i(); \
	emit32(immediate); \

// Few more

#define OP1R(op, opcode) \
	void emit_##op##_r() \
	{ \
		emit(opcode); \
	} \

#define OP1M(op, opcode) \
	void emit_##op##_rm() \
	{ \
		emit(opcode); \
	} \

#define OP1I(op, opcode, extension) \
	void emit_##op##_i() \
	{ \
		emit(opcode); \
	} \
	enum { extension_##op##_x = extension };

#define OP1X(op, opcode, extension) \
	void emit_##op##_x() \
	{ \
		emit(opcode); \
	} \
	enum { extension_##op##_x = extension }; \

OP1R(add, 0x03);
OP1M(add, 0x01);
OP1I(add, 0x81, 0x00);
OP1I(mov, 0xC7, 0x00);
OP1I(sub, 0x81, 0x00);
OP1I(jmp, 0xE9, 0x00);
OP1I(cmp, 0x3D, 0x00);
OP1X(mul, 0xF7, RSP);
OP1X(div, 0xF7, RSI);

void test_emit()
{
	for (uint8_t dst = RAX; dst <= R15; dst++)
	{
		emit_i(jmp, 0xffffffff);
		emit_i(cmp, 0x1);
		for (uint8_t src = RAX; src <= R15; src++)
		{
			if ((src & 7) != RBP && (src & 7) != RSP)
			{
				emit_md_i(add, src, 0x12345678, 0xdeadbeef);
				emit_r_m(add, dst, src);
				emit_r_md(add, dst, src, 0x12345678);
				emit_r_sib(add, dst, X4, src, dst);
				emit_r_sibd(add, dst, X4, src, dst, 0x12345678);

			}
			if ((dst & 7) != RBP && (dst & 7) != RSP)
			{
				// emit_m_r(add, dst, src);
				emit_sib_r(add, dst, X4, src, src);
				// emit_sib_i(add, dst, X4, src, 0xdeadbeef);
				emit_sibd_i(add, dst, X4, src, 0x12345678, 0xdeadbeef);
				emit_x_m(mul, dst);
				emit_x_m(div, dst);
				emit_x_d(mul, 0x12345678);
				emit_x_md(mul, dst, src, 0x12345678);
				emit_x_md(div, dst, src, 0x12345678);
			}

			emit_x_r(mul, dst);
			emit_x_r(div, dst);
			emit_r_r(add, dst, src);
			// emit_r_i(add, dst, 0x12345678);
		}
	}

	FILE *f = fopen("dump.bin", "wb");
	if (!f)
		exit(1);
	fwrite(code, emit_ptr - code, 1, f);
	fclose(f);
}

void emit_code()
{
	FILE *f = fopen("dump.bin", "wb");
	if (!f)
		exit(1);
	fwrite(code, emit_ptr - code, 1, f);
	fclose(f);
}
