/*
AppleWin : An Apple //e emulator for Windows

Copyright (C) 1994-1996, Michael O'Brien
Copyright (C) 1999-2001, Oliver Schmidt
Copyright (C) 2002-2005, Tom Charlesworth
Copyright (C) 2006-2011, Tom Charlesworth, Michael Pohoreski

AppleWin is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppleWin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppleWin; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#if CPU_TRACING // APPLE2IX
#include <assert.h>
const BYTE opcodes_65c02_numargs[256] =
{
	0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 2, 0, 0, 2, 2, 2, 0, 2, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 2, 0, 0, 2, 2, 2, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 2, 0, 0, 0, 2, 2, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 2, 0, 0, 2, 2, 2, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 2, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 2, 0, 0, 2, 2, 2, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 2, 0, 0, 0, 2, 2, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 2, 0, 0, 0, 2, 2, 0,
};
#endif

//===========================================================================

static DWORD Cpu65C02(DWORD uTotalCycles, const bool bVideoUpdate)
{
	// Optimisation:
	// . Copy the global /regs/ vars to stack-based local vars
	//   (Oliver Schmidt says this gives a performance gain, see email - The real deal: "1.10.5")
	WORD addr;
	BOOL flagc; // must always be 0 or 1, no other values allowed
	BOOL flagn; // must always be 0 or 0x80.
	BOOL flagv; // any value allowed
	BOOL flagz; // any value allowed
	WORD temp;
	WORD temp2;
	WORD val;
	AF_TO_EF
	ULONG uExecutedCycles = 0;
	WORD base;

#if CPU_TRACING // APPLE2IX
    addr = 0xFFFF;
	static FILE *trace_fp = NULL;
	BYTE valz[2];
	valz[0] = 0x00;
	valz[1] = 0x00;
	int arg_count = 0;
	if (!trace_fp) {
		trace_fp = fopen("C:\\Users\\asc\\Desktop\\cputrace.txt", "w");
	}
	unsigned int traceCycles = 0;
	static unsigned int carryoverCycles = 0;
#	define _CYC(a) CYC(a) traceCycles = (a)+uExtraCycles;
#else
#	define _CYC(a) CYC(a)
#endif

	do
	{
		UINT uExtraCycles = 0;
		BYTE iOpcode;

// NTSC_BEGIN
		ULONG uPreviousCycles = uExecutedCycles;
// NTSC_END

		if (GetActiveCpu() == CPU_Z80)
		{
			const UINT uZ80Cycles = z80_mainloop(uTotalCycles, uExecutedCycles); CYC(uZ80Cycles)
		}
		else
		{
			Fetch(iOpcode, uExecutedCycles);
#if CPU_TRACING // APPLE2IX
            fprintf(trace_fp, "%04X:%02X", regs.pc-1, iOpcode);
            arg_count = opcodes_65c02_numargs[iOpcode];
            if (arg_count == 1) {
                    Fetch(valz[0], uExecutedCycles);
                    --regs.pc;
            }
            else if (arg_count == 2) {
                    Fetch(valz[0], uExecutedCycles);
                    Fetch(valz[1], uExecutedCycles);
                    regs.pc -= 2;
            }
#endif

//#define $ INV // INV = Invalid -> Debugger Break
#define $
			switch (iOpcode)
			{
			case 0x00:              BRK  _CYC(7)  break;
			case 0x01:   idx        ORA  _CYC(6)  break;
			case 0x02: $ IMM        NOP  _CYC(2)  break;
			case 0x03: $            NOP  _CYC(1)  break;
			case 0x04:   ZPG        TSB  _CYC(5)  break;
			case 0x05:   ZPG        ORA  _CYC(3)  break;
			case 0x06:   ZPG        ASLc _CYC(5)  break;
			case 0x07: $            NOP  _CYC(1)  break;
			case 0x08:              PHP  _CYC(3)  break;
			case 0x09:   IMM        ORA  _CYC(2)  break;
			case 0x0A:              asl  _CYC(2)  break;
			case 0x0B: $            NOP  _CYC(1)  break;
			case 0x0C:   ABS        TSB  _CYC(6)  break;
			case 0x0D:   ABS        ORA  _CYC(4)  break;
			case 0x0E:   ABS        ASLc _CYC(6)  break;
			case 0x0F: $            NOP  _CYC(1)  break;
			case 0x10:   REL        BPL  _CYC(2)  break;
			case 0x11:   INDY_OPT   ORA  _CYC(5)  break;
			case 0x12:   izp        ORA  _CYC(5)  break;
			case 0x13: $            NOP  _CYC(1)  break;
			case 0x14:   ZPG        TRB  _CYC(5)  break;
			case 0x15:   zpx        ORA  _CYC(4)  break;
			case 0x16:   zpx        ASLc _CYC(6)  break;
			case 0x17: $            NOP  _CYC(1)  break;
			case 0x18:              CLC  _CYC(2)  break;
			case 0x19:   ABSY_OPT   ORA  _CYC(4)  break;
			case 0x1A:              INA  _CYC(2)  break;
			case 0x1B: $            NOP  _CYC(1)  break;
			case 0x1C:   ABS        TRB  _CYC(6)  break;
			case 0x1D:   ABSX_OPT   ORA  _CYC(4)  break;
			case 0x1E:   ABSX_OPT   ASLc _CYC(6)  break;
			case 0x1F: $            NOP  _CYC(1)  break;
			case 0x20:   ABS        JSR  _CYC(6)  break;
			case 0x21:   idx        AND  _CYC(6)  break;
			case 0x22: $ IMM        NOP  _CYC(2)  break;
			case 0x23: $            NOP  _CYC(1)  break;
			case 0x24:   ZPG        BIT  _CYC(3)  break;
			case 0x25:   ZPG        AND  _CYC(3)  break;
			case 0x26:   ZPG        ROLc _CYC(5)  break;
			case 0x27: $            NOP  _CYC(1)  break;
			case 0x28:              PLP  _CYC(4)  break;
			case 0x29:   IMM        AND  _CYC(2)  break;
			case 0x2A:              rol  _CYC(2)  break;
			case 0x2B: $            NOP  _CYC(1)  break;
			case 0x2C:   ABS        BIT  _CYC(4)  break;
			case 0x2D:   ABS        AND  _CYC(4)  break;
			case 0x2E:   ABS        ROLc _CYC(6)  break;
			case 0x2F: $            NOP  _CYC(1)  break;
			case 0x30:   REL        BMI  _CYC(2)  break;
			case 0x31:   INDY_OPT   AND  _CYC(5)  break;
			case 0x32:   izp        AND  _CYC(5)  break;
			case 0x33: $            NOP  _CYC(1)  break;
			case 0x34:   zpx        BIT  _CYC(4)  break;
			case 0x35:   zpx        AND  _CYC(4)  break;
			case 0x36:   zpx        ROLc _CYC(6)  break;
			case 0x37: $            NOP  _CYC(1)  break;
			case 0x38:              SEC  _CYC(2)  break;
			case 0x39:   ABSY_OPT   AND  _CYC(4)  break;
			case 0x3A:              DEA  _CYC(2)  break;
			case 0x3B: $            NOP  _CYC(1)  break;
			case 0x3C:   ABSX_OPT   BIT  _CYC(4)  break;
			case 0x3D:   ABSX_OPT   AND  _CYC(4)  break;
			case 0x3E:   ABSX_OPT   ROLc _CYC(6)  break;
			case 0x3F: $            NOP  _CYC(1)  break;
			case 0x40:              RTI  _CYC(6)  DoIrqProfiling(uExecutedCycles); break;
			case 0x41:   idx        EOR  _CYC(6)  break;
			case 0x42: $ IMM        NOP  _CYC(2)  break;
			case 0x43: $            NOP  _CYC(1)  break;
			case 0x44: $ ZPG        NOP  _CYC(3)  break;
			case 0x45:   ZPG        EOR  _CYC(3)  break;
			case 0x46:   ZPG        LSRc _CYC(5)  break;
			case 0x47: $            NOP  _CYC(1)  break;
			case 0x48:              PHA  _CYC(3)  break;
			case 0x49:   IMM        EOR  _CYC(2)  break;
			case 0x4A:              lsr  _CYC(2)  break;
			case 0x4B: $            NOP  _CYC(1)  break;
			case 0x4C:   ABS        JMP  _CYC(3)  break;
			case 0x4D:   ABS        EOR  _CYC(4)  break;
			case 0x4E:   ABS        LSRc _CYC(6)  break;
			case 0x4F: $            NOP  _CYC(1)  break;
			case 0x50:   REL        BVC  _CYC(2)  break;
			case 0x51:   INDY_OPT   EOR  _CYC(5)  break;
			case 0x52:   izp        EOR  _CYC(5)  break;
			case 0x53: $            NOP  _CYC(1)  break;
			case 0x54: $ zpx        NOP  _CYC(4)  break;
			case 0x55:   zpx        EOR  _CYC(4)  break;
			case 0x56:   zpx        LSRc _CYC(6)  break;
			case 0x57: $            NOP  _CYC(1)  break;
			case 0x58:              CLI  _CYC(2)  break;
			case 0x59:   ABSY_OPT   EOR  _CYC(4)  break;
			case 0x5A:              PHY  _CYC(3)  break;
			case 0x5B: $            NOP  _CYC(1)  break;
			case 0x5C: $ ABS        NOP  _CYC(8)  break;
			case 0x5D:   ABSX_OPT   EOR  _CYC(4)  break;
			case 0x5E:   ABSX_OPT   LSRc _CYC(6)  break;
			case 0x5F: $            NOP  _CYC(1)  break;
			case 0x60:              RTS  _CYC(6)  break;
			case 0x61:   idx        ADCc _CYC(6)  break;
			case 0x62: $ IMM        NOP  _CYC(2)  break;
			case 0x63: $            NOP  _CYC(1)  break;
			case 0x64:   ZPG        STZ  _CYC(3)  break;
			case 0x65:   ZPG        ADCc _CYC(3)  break;
			case 0x66:   ZPG        RORc _CYC(5)  break;
			case 0x67: $            NOP  _CYC(1)  break;
			case 0x68:              PLA  _CYC(4)  break;
			case 0x69:   IMM        ADCc _CYC(2)  break;
			case 0x6A:              ror  _CYC(2)  break;
			case 0x6B: $            NOP  _CYC(1)  break;
			case 0x6C:   IABS_CMOS  JMP  _CYC(6)  break;
			case 0x6D:   ABS        ADCc _CYC(4)  break;
			case 0x6E:   ABS        RORc _CYC(6)  break;
			case 0x6F: $            NOP  _CYC(1)  break;
			case 0x70:   REL        BVS  _CYC(2)  break;
			case 0x71:   INDY_OPT   ADCc _CYC(5)  break;
			case 0x72:   izp        ADCc _CYC(5)  break;
			case 0x73: $            NOP  _CYC(1)  break;
			case 0x74:   zpx        STZ  _CYC(4)  break;
			case 0x75:   zpx        ADCc _CYC(4)  break;
			case 0x76:   zpx        RORc _CYC(6)  break;
			case 0x77: $            NOP  _CYC(1)  break;
			case 0x78:              SEI  _CYC(2)  break;
			case 0x79:   ABSY_OPT   ADCc _CYC(4)  break;
			case 0x7A:              PLY  _CYC(4)  break;
			case 0x7B: $            NOP  _CYC(1)  break;
			case 0x7C:   IABSX      JMP  _CYC(6)  break;
			case 0x7D:   ABSX_OPT   ADCc _CYC(4)  break;
			case 0x7E:   ABSX_OPT   RORc _CYC(6)  break;
			case 0x7F: $            NOP  _CYC(1)  break;
			case 0x80:   REL        BRA  _CYC(2)  break;
			case 0x81:   idx        STA  _CYC(6)  break;
			case 0x82: $ IMM        NOP  _CYC(2)  break;
			case 0x83: $            NOP  _CYC(1)  break;
			case 0x84:   ZPG        STY  _CYC(3)  break;
			case 0x85:   ZPG        STA  _CYC(3)  break;
			case 0x86:   ZPG        STX  _CYC(3)  break;
			case 0x87: $            NOP  _CYC(1)  break;
			case 0x88:              DEY  _CYC(2)  break;
			case 0x89:   IMM        BITI _CYC(2)  break;
			case 0x8A:              TXA  _CYC(2)  break;
			case 0x8B: $            NOP  _CYC(1)  break;
			case 0x8C:   ABS        STY  _CYC(4)  break;
			case 0x8D:   ABS        STA  _CYC(4)  break;
			case 0x8E:   ABS        STX  _CYC(4)  break;
			case 0x8F: $            NOP  _CYC(1)  break;
			case 0x90:   REL        BCC  _CYC(2)  break;
			case 0x91:   INDY_CONST STA  _CYC(6)  break;
			case 0x92:   izp        STA  _CYC(5)  break;
			case 0x93: $            NOP  _CYC(1)  break;
			case 0x94:   zpx        STY  _CYC(4)  break;
			case 0x95:   zpx        STA  _CYC(4)  break;
			case 0x96:   zpy        STX  _CYC(4)  break;
			case 0x97: $            NOP  _CYC(1)  break;
			case 0x98:              TYA  _CYC(2)  break;
			case 0x99:   ABSY_CONST STA  _CYC(5)  break;
			case 0x9A:              TXS  _CYC(2)  break;
			case 0x9B: $            NOP  _CYC(1)  break;
			case 0x9C:   ABS        STZ  _CYC(4)  break;
			case 0x9D:   ABSX_CONST STA  _CYC(5)  break;
			case 0x9E:   ABSX_CONST STZ  _CYC(5)  break;
			case 0x9F: $            NOP  _CYC(1)  break;
			case 0xA0:   IMM        LDY  _CYC(2)  break;
			case 0xA1:   idx        LDA  _CYC(6)  break;
			case 0xA2:   IMM        LDX  _CYC(2)  break;
			case 0xA3: $            NOP  _CYC(1)  break;
			case 0xA4:   ZPG        LDY  _CYC(3)  break;
			case 0xA5:   ZPG        LDA  _CYC(3)  break;
			case 0xA6:   ZPG        LDX  _CYC(3)  break;
			case 0xA7: $            NOP  _CYC(1)  break;
			case 0xA8:              TAY  _CYC(2)  break;
			case 0xA9:   IMM        LDA  _CYC(2)  break;
			case 0xAA:              TAX  _CYC(2)  break;
			case 0xAB: $            NOP  _CYC(1)  break;
			case 0xAC:   ABS        LDY  _CYC(4)  break;
			case 0xAD:   ABS        LDA  _CYC(4)  break;
			case 0xAE:   ABS        LDX  _CYC(4)  break;
			case 0xAF: $            NOP  _CYC(1)  break;
			case 0xB0:   REL        BCS  _CYC(2)  break;
			case 0xB1:   INDY_OPT   LDA  _CYC(5)  break;
			case 0xB2:   izp        LDA  _CYC(5)  break;
			case 0xB3: $            NOP  _CYC(1)  break;
			case 0xB4:   zpx        LDY  _CYC(4)  break;
			case 0xB5:   zpx        LDA  _CYC(4)  break;
			case 0xB6:   zpy        LDX  _CYC(4)  break;
			case 0xB7: $            NOP  _CYC(1)  break;
			case 0xB8:              CLV  _CYC(2)  break;
			case 0xB9:   ABSY_OPT   LDA  _CYC(4)  break;
			case 0xBA:              TSX  _CYC(2)  break;
			case 0xBB: $            NOP  _CYC(1)  break;
			case 0xBC:   ABSX_OPT   LDY  _CYC(4)  break;
			case 0xBD:   ABSX_OPT   LDA  _CYC(4)  break;
			case 0xBE:   ABSY_OPT   LDX  _CYC(4)  break;
			case 0xBF: $            NOP  _CYC(1)  break;
			case 0xC0:   IMM        CPY  _CYC(2)  break;
			case 0xC1:   idx        CMP  _CYC(6)  break;
			case 0xC2: $ IMM        NOP  _CYC(2)  break;
			case 0xC3: $            NOP  _CYC(1)  break;
			case 0xC4:   ZPG        CPY  _CYC(3)  break;
			case 0xC5:   ZPG        CMP  _CYC(3)  break;
			case 0xC6:   ZPG        DEC  _CYC(5)  break;
			case 0xC7: $            NOP  _CYC(1)  break;
			case 0xC8:              INY  _CYC(2)  break;
			case 0xC9:   IMM        CMP  _CYC(2)  break;
			case 0xCA:              DEX  _CYC(2)  break;
			case 0xCB: $            NOP  _CYC(1)  break;
			case 0xCC:   ABS        CPY  _CYC(4)  break;
			case 0xCD:   ABS        CMP  _CYC(4)  break;
			case 0xCE:   ABS        DEC  _CYC(6)  break;
			case 0xCF: $            NOP  _CYC(1)  break;
			case 0xD0:   REL        BNE  _CYC(2)  break;
			case 0xD1:   INDY_OPT   CMP  _CYC(5)  break;
			case 0xD2:   izp        CMP  _CYC(5)  break;
			case 0xD3: $            NOP  _CYC(1)  break;
			case 0xD4: $ zpx        NOP  _CYC(4)  break;
			case 0xD5:   zpx        CMP  _CYC(4)  break;
			case 0xD6:   zpx        DEC  _CYC(6)  break;
			case 0xD7: $            NOP  _CYC(1)  break;
			case 0xD8:              CLD  _CYC(2)  break;
			case 0xD9:   ABSY_OPT   CMP  _CYC(4)  break;
			case 0xDA:              PHX  _CYC(3)  break;
			case 0xDB: $            NOP  _CYC(1)  break;
			case 0xDC: $ ABS        LDD  _CYC(4)  break;
			case 0xDD:   ABSX_OPT   CMP  _CYC(4)  break;
			case 0xDE:   ABSX_CONST DEC  _CYC(7)  break;
			case 0xDF: $            NOP  _CYC(1)  break;
			case 0xE0:   IMM        CPX  _CYC(2)  break;
			case 0xE1:   idx        SBCc _CYC(6)  break;
			case 0xE2: $ IMM        NOP  _CYC(2)  break;
			case 0xE3: $            NOP  _CYC(1)  break;
			case 0xE4:   ZPG        CPX  _CYC(3)  break;
			case 0xE5:   ZPG        SBCc _CYC(3)  break;
			case 0xE6:   ZPG        INC  _CYC(5)  break;
			case 0xE7: $            NOP  _CYC(1)  break;
			case 0xE8:              INX  _CYC(2)  break;
			case 0xE9:   IMM        SBCc _CYC(2)  break;
			case 0xEA:              NOP  _CYC(2)  break;
			case 0xEB: $            NOP  _CYC(1)  break;
			case 0xEC:   ABS        CPX  _CYC(4)  break;
			case 0xED:   ABS        SBCc _CYC(4)  break;
			case 0xEE:   ABS        INC  _CYC(6)  break;
			case 0xEF: $            NOP  _CYC(1)  break;
			case 0xF0:   REL        BEQ  _CYC(2)  break;
			case 0xF1:   INDY_OPT   SBCc _CYC(5)  break;
			case 0xF2:   izp        SBCc _CYC(5)  break;
			case 0xF3: $            NOP  _CYC(1)  break;
			case 0xF4: $ zpx        NOP  _CYC(4)  break;
			case 0xF5:   zpx        SBCc _CYC(4)  break;
			case 0xF6:   zpx        INC  _CYC(6)  break;
			case 0xF7: $            NOP  _CYC(1)  break;
			case 0xF8:              SED  _CYC(2)  break;
			case 0xF9:   ABSY_OPT   SBCc _CYC(4)  break;
			case 0xFA:              PLX  _CYC(4)  break;
			case 0xFB: $            NOP  _CYC(1)  break;
			case 0xFC: $ ABS        LDD  _CYC(4)  break;
			case 0xFD:   ABSX_OPT   SBCc _CYC(4)  break;
			case 0xFE:   ABSX_CONST INC  _CYC(7)  break;
			case 0xFF: $            NOP  _CYC(1)  break;
			}

#if CPU_TRACING // APPLE2IX
			{
				if (arg_count == 0) {
					fprintf(trace_fp, "    ");
				}
				else if (arg_count == 1) {
					fprintf(trace_fp, "%02X  ", valz[0]);
				}
				else if (arg_count == 2) {
					fprintf(trace_fp, "%02X%02X", valz[0], valz[1]);
				}
				else {
					assert(false && "WTF?");
				}

				fprintf(trace_fp, " SP:%02X X:%02X Y:%02X A:%02X", (BYTE)(regs.sp), regs.x, regs.y, regs.a);

				int myflagc = (regs.ps & AF_CARRY);
				int myflagx = (regs.ps & AF_RESERVED);
				int myflagi = (regs.ps & AF_INTERRUPT);
				int myflagv = (regs.ps & AF_OVERFLOW);
				int myflagb = (regs.ps & AF_BREAK);
				int myflagd = (regs.ps & AF_DECIMAL);
				int myflagz = (regs.ps & AF_ZERO);
				int myflagn = (regs.ps & AF_SIGN);

				char flags_buf[9];
				memset(flags_buf, '-', 9);
				flags_buf[8] = '\0';
				if (flagc) {
					flags_buf[0] = 'C';
				}
				if (myflagx) {
					flags_buf[1] = 'X';
				}
				if (myflagi) {
					flags_buf[2] = 'I';
				}
				if (flagv) {
					flags_buf[3] = 'V';
				}
				if (myflagb) {
					flags_buf[4] = 'B';
				}
				if (myflagd) {
					flags_buf[5] = 'D';
				}
				if (flagz) {
					flags_buf[6] = 'Z';
				}
				if (flagn) {
					flags_buf[7] = 'N';
				}

				fprintf(trace_fp, " %s", flags_buf);
				CpuCalcCycles(uExecutedCycles);
				traceCycles += carryoverCycles;
				if (traceCycles < 10) {
					fprintf(trace_fp, " CYC:%u", traceCycles/*, g_nCumulativeCycles*/);
				}
				else {
					fprintf(trace_fp, " CY:%u", traceCycles/*, g_nCumulativeCycles*/);
				}
				fprintf(trace_fp, " irqChk:%d", g_nIrqCheckTimeout);
				fprintf(trace_fp, " totCyc:%d", g_nCumulativeCycles);

				fprintf(trace_fp, "%s", "\n");
				arg_count = 0;
			}
#endif

#undef $
		}

		CheckInterruptSources(uExecutedCycles);
		NMI(uExecutedCycles, flagc, flagn, flagv, flagz);
#if CPU_TRACING
		if (g_bmIRQ && !(regs.ps & AF_INTERRUPT)) {
			fprintf(trace_fp, "IRQ:%02X\n", (uint8_t) g_bmIRQ<<3);
			//carryoverCycles = 7; -- FIXED NOW?
		}
		else {
			//carryoverCycles = 0;
		}
#endif
		IRQ(uExecutedCycles, flagc, flagn, flagv, flagz);

// NTSC_BEGIN
		if ( bVideoUpdate )
		{
			ULONG uElapsedCycles = uExecutedCycles - uPreviousCycles;
			NTSC_VideoUpdateCycles( uElapsedCycles );
		}
// NTSC_END

	} while (uExecutedCycles < uTotalCycles);

	EF_TO_AF // Emulator Flags to Apple Flags

	return uExecutedCycles;
}

//===========================================================================
