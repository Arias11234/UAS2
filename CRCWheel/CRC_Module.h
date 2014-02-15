// This file is part of UASv1.
//
// UASv1 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// UASv1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with UASv1; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

extern void SubTableEntry2__IncrementSeqnum_and_get_xorval (void);
extern void SubTableEntry2__Get_xorval_from_table3 (void);
extern void SubTableEntry2__Constructor (void);
extern void SubTableEntry3__Fetch_XorVal (void);
extern void SubTableEntry3__Constructor (void);
extern void SubTableEntry3__XOR_LOOP1 (void);
extern void SubTableEntry3__Final_INIT_Stage (void);
extern void SubTableEntry3__Crazy_XOR_00 (void);
extern void SubTableEntry3__Crazy_XOR_01 (void);
extern void SubTableEntry3__Fill_Out_Tables (void);
extern void SubTableEntry3__Fill_Out_Tables_Part2 (void);
extern void SubTableEntry3__DESTRUCTOR (void);
extern void SubTableEntry4__Constructor (void);

extern DWORD GetSendXORVal(DWORD* lpdwSendCRC );
extern void GenerateCRCs(DWORD dwSendSeed, DWORD dwRecvSeed, DWORD* lpdwSendSeed, DWORD* lpdwRecvSeed);
