/* PowerBudget v0.00.01a 2024/09/08 calculate power dissipation and budget */
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

#define DefCliIniFile    "powerb.ini"     // default filename for input with node graph
#define DefCliIniResFile "powerb.res.ini" // default filename used as output by the CLI
#define DefGuiIniResFile "powerb.GUI.ini" // default filename used as output by the GUI
#define MaxIns  3 // number of max input supply for a load, count from 0
#define MaxOut 17 // 16 number of max load for a supply, count from 0
#define MaxRserie 4 // number of max R in serie
#define MaxRsValue 10 // maximum Ohmic value for series resistors

typedef struct nTy { char name[5]; // "IN", "SRxx", "LRxx", "LDxx"
                     int type;     // IN=0, SR=1, LR=2, RS=4, LD=3
                     char label[15]; // any user string
                     char refdes[7]; // "Uxx" or "RNxxxx"
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
                     int col; // used for GUI positioning
                     int row; // used for GUI positioning
                     struct nTy* prev;
                     struct nTy* next;
                   } nTy;

typedef struct nList { // needed to support GUI
    nTy* first;
    nTy* last;
    int nodeCnt;
    int init;
} nListTy;

extern nListTy nList; // double linked list of node values

extern nTy* nPtr; // struct of nodes ptr


void nListInit(nListTy* nListPtr); // init the double linked node list

nTy* nListAdd(nListTy* nListPtr); // add an empty node to the double linked list as last element, return its pointer

void nListDel(nListTy* nListPtr, nTy* nodePtr); // delete a node from the double linked list

int loadINI(char* graphFile); // LIB: load INI file

int clearNodes(); // clear node Vi, Pd and Io

int calcNodes(); // LIB: calc nodes

int showStructData(); // show struct data

int saveINI(char* fileName); // LIB: save INI with results

int freeMem(); // LIB: free mem

#endif /* POWERB_H_ */
