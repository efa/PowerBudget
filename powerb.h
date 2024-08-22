/* PowerBudget v0.00.01a 2024/08/20 calculate power dissipation and budget */
/* Copyright 2024 Valerio Messina http://users.iol.it/efa              */
/* powerb.h is part of PowerBudget
   PowerBudget is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PowerBudget is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PowerBudget. If not, see <http://www.gnu.org/licenses/>. */

/* powerb.h CLI interface: used by CLI and GUI to fill struct and write INI files */

#ifndef POWERB_H_
#define POWERB_H_

#define DefIniFile    "powerb.ini"     // default filename for input with node graph
#define DefIniResFile "powerb.res.ini" // default filename for output with results
#define MaxIns  3 // number of max input supply for a load, count from 0
#define MaxOut 17 // 16 number of max load for a supply, count from 0
#define MaxRserie 4 // number of max R in serie

typedef struct nTy { char name[5]; // "IN", "SRxx", "LRxx", "LDxx"
                     int type;     // IN=0, SR=1, LR=2, RS=4, LD=3
                     char label[15]; // any user string
                     char refdes[7]; // "Uxx" or "RNxxxx"
                     int col; // used for positioning
                     int row; // used for positioning
                     struct nTy* from[MaxIns]; // SR,LR,RS has 1, LD up to MaxIns
                     char in[MaxIns][5]; // used by the GUI
                     double Vi[MaxIns];
                     double Ii[MaxIns];
                     double R[MaxIns];
                     double Pi[MaxIns];
                     double eta;
                     double Iadj;
                     double DV;
                     double Pd;
                     double Vo;
                     double Io;
                     double Po;
                     struct nTy* to[MaxOut]; // to leaf
                     int out;
                   } nTy;

int initNodeData(nTy* node);

int fillNodeData(int id, nTy* node);

int saveINI(void* nodedit);

int calcINI();

#endif /* POWERB_H_ */
