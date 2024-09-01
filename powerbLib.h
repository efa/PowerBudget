/* PowerBudget v0.00.01a 2024/08/20 calculate power dissipation and budget */
/* Copyright 2024 Valerio Messina http://users.iol.it/efa              */
/* powerbLib.h is part of PowerBudget
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

/* powerbLib.h LIB interface: used by CLI/GUI to fill struct and write INI files */

#ifndef POWERB_H_
#define POWERB_H_

#define DefIniFile    "powerb.ini"     // default filename for input with node graph
#define DefIniResFile "powerb.res.ini" // default filename for output with results
#define MaxIns  3 // number of max input supply for a load, count from 0
#define MaxOut 17 // 16 number of max load for a supply, count from 0
#define MaxRserie 4 // number of max R in serie
#define MaxRsValue 10 // maximum Ohmic value for series resistors

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
                     double yeld;
                     double Iadj;
                     double DV;
                     double Pd;
                     double Vo;
                     double Io;
                     double Po;
                     struct nTy* to[MaxOut]; // to leaf
                     int out;
                   } nTy;

extern nTy* nPtr; // struct of nodes ptr


int saveINI(void* nodedit); // GUI: before call "powerb"

int calcINI(); // GUI: call to "powerb"

int loadINIres(void* nodedit); // GUI: 

int initNodeData(nTy* node); // GUI: 

int fillNodeData(int id, nTy* node); // GUI: 

int loadINI(char* graphFile, int* sectPtr); // LIB: load INI file

int calcNodes(); // LIB: calc nodes

int saveINIres(nTy* nPtr, int nodes); // LIB: save INI with results

int freeMem(); // LIB: free mem

#endif /* POWERB_H_ */
