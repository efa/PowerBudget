/* PowerBudget v0.00.01a 2024/09/08 calculate power dissipation and budget */
/* Copyright 2024 Valerio Messina http://users.iol.it/efa              */
/* powerbLib.c is part of PowerBudget
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

/* powerbLib.c LIB: read INI file, check content and fill struct */

#include <stdio.h>
#include <strings.h>

#include <iniparser.h> // dictionary with N sections with M keys

#include "powerbLib.h"
#include "fileIo.h"

nTy* nPtr; // vector of struct/nodes ptr
int sect;  // number of sections/nodes
dictionary* graphPtr; // INI file dictionary ptr

nTy* missFrom=NULL; // take note of node to complete later

int loadINI(char* graphFile, int* sectPtr) {
   // parse ini file
   graphPtr=iniparser_load(graphFile);
   if (graphPtr==NULL) {
      printf("Cannot open and parse file:'%s'. Quit\n", graphFile);
      return -1;
   }
   sect=iniparser_getnsec(graphPtr);
   //printf("sect:%d\n", sect);
   if (sect<3) { // INI sections
      printf("Too few sections in file. Quit\n");
      return -1;
   }

   // check needed sections/nodes
   int board=0; int in=0; int sr=0; int lr=0; int rs=0; int ld=0;
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      const char* sectNamePtr=iniparser_getsecname(graphPtr, s);
      //printf("s:%d name:'%s'\n", s, sectNamePtr);
      if (strcasecmp(sectNamePtr, "board")==0) board++;
      if (strcasecmp(sectNamePtr, "in")==0) in++;
      char noPtr[3];
      strncpy(noPtr, sectNamePtr, 2); noPtr[2]='\0';
      if (strcasecmp(noPtr, "sr")==0) sr++;
      if (strcasecmp(noPtr, "lr")==0) lr++;
      if (strcasecmp(noPtr, "rs")==0) rs++;
      if (strcasecmp(noPtr, "ld")==0) ld++;
      iniparser_getsecnkeys(graphPtr, sectNamePtr);
      //printf("keys:%d\n", keys);
   }
   if (board==0) {
      printf("Missing BOARD section in file. Quit\n");
      return -1;
   }
   if (board>1) {
      printf("Only one BOARD is allowed in file. Quit\n");
      return -1;
   }
   if (in==0) {
      printf("Missing IN section in file. Quit\n");
      return -1;
   }
   if (in>1) {
      printf("Only one IN is allowed in file. Quit\n");
      return -1;
   }
   if (ld==0) {
      printf("Missing LD1 section in file. Quit\n");
      return -1;
   }
   printf("INI file:'%s'\n", graphFile);
   printf("BOARD in file:'%s'\n", iniparser_getstring(graphPtr, "BOARD:label", ""));
   printf("Input in file:%d\n", in);
   printf("Switching Regulators in file:%d\n", sr);
   printf("Linear Regulators in file:%d\n", lr);
   printf("Resistor serie in file:%d\n", rs);
   printf("Loads in file:%d\n", ld);
   int nt=in+sr+lr+rs+ld;
   //printf("Tot Sect:%d Nodes:%d\n", sect, nt);
   printf("\n");

   // allocate space for nodes
   nPtr = malloc((nt+1)*sizeof(nTy)); // keep space for BOARD in [0]

   // fill struct with file data and check valid values
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      //printf("s:%d\n", s);
      const char* sectNamePtr=iniparser_getsecname(graphPtr, s);
      if (strcasecmp(sectNamePtr, "board")==0) { // BOARD only
         strcpy(nPtr[s].name, sectNamePtr);
         nPtr[s].type=-1;
         strcpy(nPtr[s].label, iniparser_getstring(graphPtr, "BOARD:label", ""));
         strcpy(nPtr[s].refdes, "");
         nPtr[s].col=-1;
         nPtr[s].row=-1;
         for (int i=0; i<MaxIns; i++) {
            nPtr[s].from[i]=NULL;
            strcpy(nPtr[s].in[i], "");
            nPtr[s].Vi[i]=0;
            nPtr[s].Ii[i]=0;
            nPtr[s].Pi[i]=0;
            nPtr[s].R[i]=0;
         }
         nPtr[s].yeld=0;
         nPtr[s].Iadj=0;
         nPtr[s].DV=0;
         nPtr[s].Pd=0;
         nPtr[s].Vo=0;
         nPtr[s].Io=0;
         nPtr[s].Po=0;
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
      }

      if (strcasecmp(sectNamePtr, "in")==0) { // IN only
         strcpy(nPtr[s].name, sectNamePtr);
         nPtr[s].type=0;
         strcpy(nPtr[s].label, iniparser_getstring(graphPtr, "IN:label", ""));
         strcpy(nPtr[s].refdes, "");
         nPtr[s].col=-1;
         nPtr[s].row=-1;
         for (int i=0; i<MaxIns; i++) {
            nPtr[s].from[i]=NULL;
            strcpy(nPtr[s].in[i], "");
            nPtr[s].Vi[i]=0;
            nPtr[s].Ii[i]=0;
            nPtr[s].Pi[i]=0;
            nPtr[s].R[i]=0;
         }
         nPtr[s].yeld=0;
         nPtr[s].Iadj=0;
         nPtr[s].DV=0;
         nPtr[s].Pd=0;
         nPtr[s].Vo=iniparser_getdouble(graphPtr, "IN:V", 0);
         nPtr[s].Io=iniparser_getdouble(graphPtr, "IN:I", 0);
         nPtr[s].Po=iniparser_getdouble(graphPtr, "IN:P", 0);
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
         if (nPtr[s].Vo==0) {
            printf("Invalid input for IN. Quit\n");
            return -1;
         }
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
      }
      char sectTypePtr[3]="";
      strncpy(sectTypePtr, sectNamePtr, 2); sectTypePtr[2]='\0';
      char sectKeyPtr[11]="";

      if (strcasecmp(sectTypePtr, "sr")==0) { // SR only
         strcpy(nPtr[s].name, sectNamePtr);
         nPtr[s].type=1;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":label");
         strcpy(nPtr[s].label, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":refdes");
         strcpy(nPtr[s].refdes, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         nPtr[s].col=-1;
         nPtr[s].row=-1;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":f0");
         const char* strPtr=iniparser_getstring(graphPtr, sectKeyPtr, NULL);
         if (strPtr==NULL) {
            printf("Node:'%s' from:NULL. Quit\n", sectNamePtr);
            return -1;
         }
         char noPtr[3];
         strncpy(noPtr, strPtr, 2); noPtr[2]='\0';
         if (strcasecmp(noPtr, "ld")==0) {
            printf("Node:'%s' from:LDn. Quit\n", sectNamePtr);
            return -1;
         }
         nPtr[s].from[0]=NULL;
         strcpy(nPtr[s].in[0], strPtr);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Vi");
         nPtr[s].Vi[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Ii");
         nPtr[s].Ii[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Pi");
         nPtr[s].Pi[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":R");
         nPtr[s].R[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         for (int i=1; i<MaxIns; i++) {
            nPtr[s].from[i]=NULL;
            strcpy(nPtr[s].in[i], "");
            nPtr[s].Vi[i]=0;
            nPtr[s].Ii[i]=0;
            nPtr[s].Pi[i]=0;
            nPtr[s].R[i]=0;
         }
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":n");
         nPtr[s].yeld=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         nPtr[s].Iadj=0;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":DV");
         nPtr[s].DV=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Pd");
         nPtr[s].Pd=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Vo");
         nPtr[s].Vo=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Io");
         nPtr[s].Io=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Po");
         nPtr[s].Po=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
         if (nPtr[s].Vo==0) {
            printf("Invalid input for SR:'%s'. Quit\n", sectNamePtr);
            return -1;
         }
      }

      if (strcasecmp(sectTypePtr, "lr")==0) { // LR only
         strcpy(nPtr[s].name, sectNamePtr);
         nPtr[s].type=2;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":label");
         strcpy(nPtr[s].label, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":refdes");
         strcpy(nPtr[s].refdes, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         nPtr[s].col=-1;
         nPtr[s].row=-1;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":f0");
         const char* strPtr=iniparser_getstring(graphPtr, sectKeyPtr, NULL);
         if (strPtr==NULL) {
            printf("Node:'%s' from:NULL. Quit\n", sectNamePtr);
            return -1;
         }
         char noPtr[3];
         strncpy(noPtr, strPtr, 2); noPtr[2]='\0';
         if (strcasecmp(noPtr, "ld")==0) {
            printf("Node:'%s' from:LDn. Quit\n", sectNamePtr);
            return -1;
         }
         nPtr[s].from[0]=NULL;
         strcpy(nPtr[s].in[0], strPtr);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Vi");
         nPtr[s].Vi[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Ii");
         nPtr[s].Ii[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Pi");
         nPtr[s].Pi[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":R");
         nPtr[s].R[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         for (int i=1; i<MaxIns; i++) {
            nPtr[s].from[i]=NULL;
            strcpy(nPtr[s].in[i], "");
            nPtr[s].Vi[i]=0;
            nPtr[s].Ii[i]=0;
            nPtr[s].Pi[i]=0;
            nPtr[s].R[i]=0;
         }
         nPtr[s].yeld=0;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Iadj");
         nPtr[s].Iadj=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":DV");
         nPtr[s].DV=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Pd");
         nPtr[s].Pd=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":n");
         nPtr[s].yeld=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Vo");
         nPtr[s].Vo=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Io");
         nPtr[s].Io=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Po");
         nPtr[s].Po=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
         if (nPtr[s].Vo==0) {
            printf("Invalid input for LR:'%s', miss Vo. Quit\n", sectNamePtr);
            return -1;
         }
      }

      if (strcasecmp(sectTypePtr, "rs")==0) { // RS only
         strcpy(nPtr[s].name, sectNamePtr);
         nPtr[s].type=4;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":label");
         strcpy(nPtr[s].label, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":refdes");
         strcpy(nPtr[s].refdes, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         nPtr[s].col=-1;
         nPtr[s].row=-1;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":f0");
         const char* strPtr=iniparser_getstring(graphPtr, sectKeyPtr, NULL);
         if (strPtr==NULL) {
            printf("Node:'%s' from:NULL. Quit\n", sectNamePtr);
            return -1;
         }
         char noPtr[3];
         strncpy(noPtr, strPtr, 2); noPtr[2]='\0';
         if (strcasecmp(noPtr, "ld")==0) {
            printf("Node:'%s' from:LDn. Quit\n", sectNamePtr);
            return -1;
         }
         nPtr[s].from[0]=NULL;
         strcpy(nPtr[s].in[0], strPtr);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Vi");
         nPtr[s].Vi[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Ii");
         nPtr[s].Ii[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Pi");
         nPtr[s].Pi[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":R");
         nPtr[s].R[0]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         for (int i=1; i<MaxIns; i++) {
            nPtr[s].from[i]=NULL;
            strcpy(nPtr[s].in[i], "");
            nPtr[s].Vi[i]=0;
            nPtr[s].Ii[i]=0;
            nPtr[s].Pi[i]=0;
            nPtr[s].R[i]=0;
         }
         nPtr[s].yeld=0;
         nPtr[s].Iadj=0;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":DV");
         nPtr[s].DV=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Pd");
         nPtr[s].Pd=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Vo");
         nPtr[s].Vo=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Io");
         nPtr[s].Io=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":Po");
         nPtr[s].Po=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
         if (nPtr[s].R[0]==0) {
            printf("Invalid input for RS:'%s', miss R. Quit\n", sectNamePtr);
            return -1;
         }
         if (nPtr[s].R[0]>MaxRsValue) {
            printf("Invalid input for RS:'%s'. Quit\n", sectNamePtr);
            return -1;
         }
      }

      if (strcasecmp(sectTypePtr,"ld")==0) { // LD only
         strcpy(nPtr[s].name, sectNamePtr);
         nPtr[s].type=3;
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":label");
         strcpy(nPtr[s].label, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":refdes");
         strcpy(nPtr[s].refdes, iniparser_getstring(graphPtr, sectKeyPtr, ""));
         nPtr[s].col=-1;
         nPtr[s].row=-1;
         for (int i=0; i<MaxIns; i++) {
            //printf("i:%d\n", i);
            char snPtr[2];
            sprintf(snPtr, "%d", i);
            strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":f"); strcat(sectKeyPtr, snPtr);
            //printf("NodeKey:'%s'\n", sectKeyPtr);
            const char* strPtr=iniparser_getstring(graphPtr, sectKeyPtr, NULL);
            if (strPtr==NULL && i==0) { // at least one input from needed
               printf("Node:'%s' from:NULL. Quit\n", sectNamePtr);
               return -1;
            }
            if (strPtr==NULL) { // at least one input from
               nPtr[s].from[i]=NULL;
               strcpy(nPtr[s].in[i], "");
               nPtr[s].Vi[i]=0;
               nPtr[s].Ii[i]=0;
               nPtr[s].Pi[i]=0;
               nPtr[s].R[i]=0;
               //printf("Node:'%s' continue\n", sectNamePtr);
               continue;
            }
            char noPtr[3];
            strncpy(noPtr, strPtr, 2); noPtr[2]='\0';
            //printf("str:'%s'\n", noPtr);
            if (strcasecmp(noPtr, "ld")==0) {
               printf("Node:'%s' from:LDn. Quit\n", sectNamePtr);
               return -1;
            }
            nPtr[s].from[i]=NULL;
            strcpy(nPtr[s].in[i], strPtr);
            strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":V"); strcat(sectKeyPtr, snPtr);
            nPtr[s].Vi[i]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
            strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":I"); strcat(sectKeyPtr, snPtr);
            nPtr[s].Ii[i]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
            strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":P"); strcat(sectKeyPtr, snPtr);
            nPtr[s].Pi[i]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
            strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":R"); strcat(sectKeyPtr, snPtr);
            nPtr[s].R[i]=iniparser_getdouble(graphPtr, sectKeyPtr, 0);
         }
         nPtr[s].yeld=0;
         nPtr[s].Iadj=0;
         nPtr[s].DV=0;
         nPtr[s].Pd=0;
         nPtr[s].Vo=0;
         nPtr[s].Io=0;
         nPtr[s].Po=0;
         for (int t=0; t<MaxOut; t++) {
            nPtr[s].to[t]=NULL;
         }
         nPtr[s].out=0;
         if (nPtr[s].Ii[0]==0 && nPtr[s].Pi[0]==0 && nPtr[s].R[0]==0) {
            printf("Invalid input for LD:'%s'. Quit\n", sectNamePtr);
            return -1;
         }
      }
   }

   // 2nd pass to check from names and fill ptrs
   //printf("2nd pass ...\n");
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      //printf("node:'%s'\n", nPtr[s].name);
      //printf("type:'%d'\n", nPtr[s].type);
      if (nPtr[s].type == 0 || nPtr[s].type == -1) continue; // skip BOARD & IN
      const char* sectNamePtr=iniparser_getsecname(graphPtr, s);
      char sectKeyPtr[10];
      for (int i=0; i<MaxIns; i++) {
         //printf("i:%d\n", i);
         char snPtr[2];
         sprintf(snPtr, "%d", i);
         strcpy(sectKeyPtr, sectNamePtr); strcat(sectKeyPtr, ":f"); strcat(sectKeyPtr, snPtr);
         //printf("NodeKey:'%s'\n", sectKeyPtr);
         const char* strPtr=iniparser_getstring(graphPtr, sectKeyPtr, NULL);
         if (strPtr==NULL && i==0) { // at least one input from
            printf("Node:'%s' from:NULL. Quit\n", sectNamePtr);
            return -1;
         }
         if (strPtr==NULL) {
            //printf("Node:'%s' continue\n", sectNamePtr);
            continue;
         }
         //printf("search from strPtr:'%s'\n", strPtr);
         if (strcasecmp(strPtr, sectNamePtr)==0) {
            printf("Node:'%s' from itself. Quit\n", sectNamePtr);
            return -1;
         }
         char noPtr[3];
         strncpy(noPtr, strPtr, 2); noPtr[2]='\0';
         if (strcasecmp(noPtr, "ld")==0) {
            printf("Node:'%s' from:LDn. Quit\n", sectNamePtr);
            return -1;
         }
         int srcOK=0;
         for (int n=0; n<sect; n++) { // check in all section if exist
            const char* nodeNamePtr=iniparser_getsecname(graphPtr, n);
            //printf("check nodeNamePtr:'%s'\n", nodeNamePtr);
            if (strcasecmp(strPtr, nodeNamePtr)==0) {
               //printf("Node:'%s' from:'%s' found\n", nPtr[s].name, strPtr);
               srcOK=1;
               nPtr[s].from[i]=&(nPtr[n]);
               // now fill to[] of from node: nPtr[n]
               for (int t=0; t<MaxOut; t++) { // find first free
                  if (nPtr[n].to[t]!=NULL) continue;
                  //printf("fill t:%d\n", t);
                  nPtr[n].to[t]=&(nPtr[s]);
                  break;
               }
               break;
            }
         }
         //printf("srcOK:%d\n", srcOK);
         if (srcOK==0) {
            printf("Node:'%s' from:'%s' not found. Quit\n", nPtr[s].name, strPtr);
            return -1;
         }
      }
   }
   //printf("\n");

   // 3rd pass to discover max depth and load input valuess
   //printf("3rd pass, discover max depth ...\n");
   int md=0;
   int ml=0;
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      if (nPtr[s].type!=3) continue; // only LDx
      //printf("s:%d node:'%s'\n", s, nPtr[s].name);
      int d=0;
      for (int i=0; i<MaxIns; i++) { // for every load input
         d=0;
         //printf("input i:%d\n", i);
         if (nPtr[s].from[i]!=NULL) {
            ml++;
            nTy* from=nPtr[s].from[i];
            //printf("from:%p\n", from);
            int type=from->type;
            //printf("type:%d\n", type);
            while (type!=0) { // up to IN
               d++;
               //printf("col:%d row:%d node:'%s' type:%d \n", d, ml, from->name, from->type);
               from=from->from[0];
               type=from->type;
            }
            //printf("col:%d row:%d node:'%s' type:%d\n", d, ml, from->name, from->type);
         }
         if (d>md) md=d; // new max depth
      } // for (int i=0; i<MaxIns; i++)
   } // for (int s=0; s<sect; s++)
   md++; ml--;
   //printf("md:%d ml:%d\n", md, ml);
   int cols=md+1, rows=ml+1;
   printf("cols:%d lines:%d\n", cols, rows);
   printf("\n");

   // 4th pass, graph exploration and fill
   //printf("graph exploration and fill\n");
   int c=0;
   int r=0;
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      //printf("start s:%d\n", s);
      if (nPtr[s].type!=3) continue; // explore only from LDx
      //printf("node:'%- 4s'\n", nPtr[s].name);
      for (int i=0; i<MaxIns; i++) { // for every load input
         //printf("start input i:%d r:%d\n", i, r);
         if (nPtr[s].from[i]!=NULL) { // only if there is a connection to this input
            //printf("c   :% 2d r  :% 2d node:'%- 4s' type:%d addr:%p\n", c, r, nPtr[s].name, nPtr[s].type, &nPtr[s]);
            //printf("col+:% 2d row:% 2d node:'%- 4s' type:%d addr:%p\n", nPtr[s].col, nPtr[s].row, nPtr[s].name, nPtr[s].type, &nPtr[s]);
            if (nPtr[s].col==-1) nPtr[s].col=c;
            if (nPtr[s].row==-1) nPtr[s].row=r;
            //printf("col+:% 2d row:% 2d node:'%- 4s' type:%d addr:%p\n", nPtr[s].col, nPtr[s].row, nPtr[s].name, nPtr[s].type, &nPtr[s]);
            c++;
            nTy* from=nPtr[s].from[i];
            //printf("start name:'%s'\n", from->name);
            int type=from->type;
            while (type!=0) { // IN
               //printf("c  _:% 2d r  :% 2d node:'%- 4s' type:%d addr:%p\n", c, r, from->name, from->type, from);
               //printf("col_:% 2d row:% 2d node:'%- 4s' type:%d addr:%p\n", from->col, from->row, from->name, from->type, from);
               if (from->col==-1 || c>from->col) from->col=c;
               if (from->row==-1) from->row=r;
               //printf("col_:% 2d row:% 2d node:'%- 4s' type:%d addr:%p\n", from->col, from->row, from->name, from->type, from);
               from=from->from[0];
               type=from->type;
               c++;
            }
            //printf("c   :% 2d r  :% 2d node:'%- 4s' type:%d addr:%p _\n", c, r, from->name, from->type, from);
            if (from->col==-1 || c>from->col) from->col=c;
            if (from->row==-1) from->row=r;
            //printf("col :% 2d row:% 2d node:'%- 4s' type:%d addr:%p\n", from->col, from->row, from->name, from->type, from);
            c++;
            r++;
         }
         //printf("end   input i:%d r:%d\n", i, r);
         c=0;
      } // for (int i=0; i<MaxIns; i++)
      //printf("end   s:%d\n", s);
   } // for (int s=0; s<sect; s++)
   //printf("\n");

#if 0
   printf("show node matrix data\n");
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      //printf("node:'%- 4s'\n", nPtr[s].name);
      int maxIn=1;
      if (nPtr[s].type==3) maxIn=MaxIns;
      for (int i=0; i<maxIn; i++) { // for every load input
         if (nPtr[s].type!=3 || nPtr[s].from[i]!=NULL) { // only if there is a connection to this LD input
            printf("node:%d input:%d name:'%s' col:%d row:%d\n", s, i, nPtr[s].name, nPtr[s].col, nPtr[s].row);
         }
      }
   }
   printf("\n");
#endif

   // empty matrix
   //printf("empty matrix\n");
   nTy* node[cols][rows]; // node Ptr [col][row] used for graph positioning
   for (int c=0; c<cols; c++) {
      for (int r=0; r<rows; r++) {
         node[c][r]=NULL;
      }
   }

   //printf("fill matrix data\n");
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      if (nPtr[s].type==-1) continue;
      //printf("node:'%- 4s'\n", nPtr[s].name);
      //printf("nPtr[s].col:%d nPtr[s].row:%d\n", nPtr[s].col, nPtr[s].row);
      node[nPtr[s].col][nPtr[s].row]=&nPtr[s]; // fill matrix
      if (nPtr[s].type==3) { // LOAD can have more than 1 input
         for (int i=1; i<MaxIns; i++) { // for every LOAD input
            if (nPtr[s].from[i]!=NULL) { // only if there is a connection to this LD input
               //printf("node:%d input:%d name:'%s' col:%d row:%d\n", s, i, nPtr[s].name, nPtr[s].col, nPtr[s].row);
               node[nPtr[s].col][nPtr[s].row+i]=&nPtr[s]; // fill matrix
            }
         }
      }
   }
   //printf("\n");

#if 0
   printf("show matrix data\n");
   for (int r=0; r<rows; r++) {
      for (int c=0; c<cols; c++) {
         if (node[c][r]!=NULL)
            printf("col:%d row:%d ptr addr:%p name:'%s'\n", c, r, node[c][r], node[c][r]->name);
      }
   }
   printf("\n");
#endif

   printf("show graph matrix\n");
   printf("rows\\cols|");
   for (int c=md; c>=0; c--) {
      printf("      %d|", c);
   }
   printf("\n");
   printf("---------+");
   for (int c=md; c>=0; c--) {
      printf("-------+");
   }
   printf("\n");
   for (int r=0; r<rows; r++) {
      printf(" %02d      |", r);
      for (int c=md; c>=0; c--) {
         if (node[c][r]!=NULL) {
            // printf("col:%d row:%d ptr addr:%p name:'%s'\n", c, r, node[c][r], node[c][r]->name);
            printf(" '%-4s'|", node[c][r]->name);
         } else {
            printf("       |");
         }
      }
      printf("\n");
   }
   printf("\n");
   *sectPtr=sect;
   return 0;
} // int loadINI(char* graphFile, int* sectPtr)

double calcP(double v, double i) { // calc power
   return v*i;
}

double calcV(double p, double i) { // calc voltage
   if (i==0) return 0;
   return p/i;
}

double calcI(double p, double v) { // calc current
   if (v==0) return 0;
   return p/v;
}

double calcR(double v, double i) { // calc resistance
   if (i==0) return 0;
   return v/i;
}

#if 0
double partV(double v, double Rt, double Ro) { // calc voltage partitor
   return v*Ro/Rt;
}

double partV(double v, double R[], int pos) { // calc voltage partitor
   double Rt=0;
   for (int p=0; p<MaxRserie; p++) {
      Rt+=R[p];
   }
   return v*R[pos]/Rt;
}
#endif

double findInputV(nTy* node) {
   if (node->from[0]->type==4) printf("WARN: RS on root path\n");
   double Vo=node->from[0]->Vo;
   return Vo;
} // double findInputV(nTy* node)

// calc all outputs for IN
int calcIN(nTy* from, double Io) {
   int ret=0;
   from->Io+=Io; // current from node below
   from->out++;
   //printf("IN from->Io:%g\n", from->Io);
   //printf("IN from->out:%d\n", from->out);
   if (from->to[from->out]==NULL) { // all out current summed
      //printf("IN out complete, calc all ...\n");
      from->Po=from->Vo*from->Io; // output power
   } else ret=1; // goto skip; // stop graph exploration to root
   return ret;
} // int calcIN(nTy* node, double Io)

// may miss Vi, in case do partial calcs
int calcSR(nTy* from, double* Io) {
   int ret=0;
   //from->Io+=from->to[0]->Ii[0]; // output current of father get from son input current
   from->Io+=*Io; // current from node below
   from->out++;
   //printf("SR from->Io:%g\n", from->Io);
   //printf("SR from->out:%d\n", from->out);
   if (from->to[from->out]==NULL) { // all out current summed
      //printf("SR out complete, calc input ...\n");
      from->Po=from->Vo*from->Io; // output power of father calculated from Vo on Io
      from->Pd=from->Po*(1/from->yeld-1);
      from->Pi[0]=from->Po/from->yeld;
      if (from->Vi[0]==0) {
         from->Vi[0]=findInputV(from);
         //printf("SR from->Vi[0]:%g\n", from->Vi[0]);
      }
      if (from->Vi[0]!=0) {
         from->DV=from->Vi[0]-from->Vo;
         from->Ii[0]=from->Pi[0]/from->Vi[0];
      } else {
         if (missFrom!=NULL) printf("WARN: SR lost node for multiple RS");
         missFrom=from; // to complete later
         ret|=2;
      }
      *Io=from->Ii[0]; // pass current to node up
      //printf("SR Ii:%g\n", *Io);
   } else ret|=1; // goto skip; // stop graph exploration to root
   return ret;
} // int calcSR(nTy* node, double* Io)

// may miss Vi, in case do partial calcs
int calcLR(nTy* from, double* Io) {
   int ret=0;
   //printf("LR i:%d from->to[0]:%p\n", i, from->to[0]);
   from->Io+=*Io; // current from node below
   from->out++;
   //printf("LR from->Io:%g\n", from->Io);
   //printf("LR from->out:%d\n", from->out);
   if (from->to[from->out]==NULL) { // all out current summed
      //printf("LR out complete, calc input ...\n");
      from->Po=from->Vo*from->Io; // output power of father calculated from Vo on Io
      from->Ii[0]=from->Io+from->Iadj;
      if (from->Vi[0]==0) {
         from->Vi[0]=findInputV(from);
         //printf("LR from->Vi[0]:%g\n", from->Vi[0]);
      }
      if (from->Vi[0]!=0) {
         from->DV=from->Vi[0]-from->Vo;
         from->Pd=from->Io*from->DV+from->Iadj*from->Vi[0];
         from->Pi[0]=from->Vi[0]*from->Ii[0];
      } else {
         if (missFrom!=NULL) printf("WARN: LR lost node for multiple RS");
         missFrom=from; // to complete later
         ret|=2;
      }
      *Io=from->Ii[0]; // pass current to node up
      //printf("LR Io:%g\n", *Io);
   } else ret|=1; // goto skip; // stop graph exploration to root
   return ret;
} // int calcLR(nTy* node, double* Io)

// may be many RS series, in case find all
int calcRS(nTy* from, double Io, double* Vo) {
   int ret=0;
   //printf("from->name:%s\n", from->name);
   //printf("RS Io:%g\n", Io);
   double Vi=0; // Voltage at input of RS series
   double Rt=0;
   int p;
   nTy* fromSave=from; // save for 2nd pass
   //printf("RS discover Rt and Vi ...\n");
   for (p=0; p<MaxRserie; p++) { // look for first regulator
      //printf("from->name:%s\n", from->name);
      int type=from->type;
      //printf("type:%d\n", type);
      if (type==4) {
         Rt+=from->R[0];
      }
      if (type==0 || type==1 || type==2) { // IN or regulators
         Vi=from->Vo;
         break;
      }
      from=from->from[0];
   } // know: Io, Vi, Rt;
   //printf("RS Vit:%g\n", Vi);
   //printf("RS DVt:%g\n", Io*Rt);
   //printf("RS RSt:%g\n", Rt);
   double Vp=Vi-Io*Rt; // Voltage after RS series
   //printf("Vo:%g output of RS series\n", Vp);
   *Vo=Vp;
   //printf("RS p:%d exploring down to fill single data ...\n", p);
   int q=p;
   // need: Io, Vi, Rt;
   from=fromSave;
   for (p=0; p<q; p++) { // all RS found
      //printf("p:%d\n", p);
      //printf("Vp:%g\n", Vp);
      //printf("from->name:%s\n", from->name);
      //printf("from->R[0]:%g\n", from->R[0]);
      from->Vo=Vp;
      from->Io=Io;
      from->Po=from->Vo*Io;
      from->DV=from->R[0]*Io; // deltaV on RS single
      from->Pd=from->DV*Io;
      from->Vi[0]=from->DV+Vp;
      from->Ii[0]=Io;
      from->Pi[0]=from->Vi[0]*Io;
      //printf("from->Vi[0]:%g\n", from->Vi[0]);
      //printf("from->DV:%g\n", from->DV);
      //printf("from->Vo:%g\n", from->Vo);
      from->out=1;
      Vp=from->Vi[0]; // take note of Voltage at RS input
      from=from->from[0];
   }
   return ret;
} // int calcRS(nTy* node, double Io)

int calcNodes() {
   // calc section
   printf("calc section ...\n");
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      //printf("node:%d type:%d\n", s, nPtr[s].type);
      if (nPtr[s].type==-1) continue; // board
      if (nPtr[s].type!=3) continue; // calc only LDx
      //printf("---\n");
      //printf("new s:%d node:'%s'\n", s, nPtr[s].name);
      for (int i=0; i<MaxIns; i++) { // for every load input
         //printf("\n");
         //printf("s:%d node:'%s' node:%p input:%d\n", s, nPtr[s].name, &nPtr[s], i);
         //char snPtr[2];
         //sprintf(snPtr, "%d", i);
         nTy* from;
         int type;
         double Vo=0;
         if (nPtr[s].Vi[i]==0) { // do not know the input voltage
            if (nPtr[s].from[i]!=NULL) { // only if there is an input connection
               type=nPtr[s].from[i]->type;
               if (type==0 || type==1 || type==2) { // IN or regulators
                  nPtr[s].Vi[i]=nPtr[s].from[i]->Vo; // copy Voltage U=>D
               } else { // found RS so need voltage partitor
                  //printf("RS processing ...\n");
                  calcRS(*nPtr[s].from, nPtr[s].Ii[i], &Vo);
#if 0
                  //printf("nPtr[s].name:%s\n", nPtr[s].name);
                  double R[MaxRserie];
                  double Rt=0;
                  from=nPtr[s].from[i];
                  //printf("nPtr[s].from[i].name:%s\n", nPtr[s].from[i]->name);
                  //printf("from.name:%s\n", from->name);
                  //R[0]=from->R[0];
                  //printf("R[0]:%g\n", R[0]);
                  double Vi; // Voltage at input of RS series
                  int p;
                  for (p=0; p<MaxRserie; p++) { // look for first regulator
                     //printf("from.name:%s\n", from->name);
                     type=from->type;
                     //printf("type:%d\n", type);
                     if (type==4) {
                        from->Io=nPtr[s].Ii[i];
                        R[p]=from->R[0];
                        //printf("R[%d]:%g\n", p, R[p]);
                        double DV=R[p]*from->Io; // deltaV on RS single
                        from->DV=DV;
                        from->Pd=DV*from->Io;
                        from->Ii[0]=from->Io;
                        Rt+=R[p];
                     }
                     if (type==0 || type==1 || type==2) { // IN or regulators
                        Vi=from->Vo;
                        //printf("RS Vi:%g\n", Vi);
                        break;
                     }
                     from=from->from[0];
                  } // know: from->Io, Ii[0], Rt, Vi; [R[p], DV, Pd]
                  //printf("RS Rt:%g\n", Rt);
                  double DV=Rt*nPtr[s].Ii[i]; // deltaV tot on RS series
                  //printf("tot DV:%g\n", DV);
                  //nPtr[s].DV=DV;
                  nPtr[s].Vi[i]=Vi-DV;
                  double Vp=nPtr[s].Vi[i]; // Voltage at RS series end
                  //printf("RS Vo:%g\n", Vp);
                  //printf("RS p:%d exploring down to fill single data ...\n", p);
                  int q=p;
                  from=nPtr[s].from[i];
                  for (p=0; p<q; p++) { // all RS found
                     //printf("from->name:%s\n", from->name);
                     //printf("from->type:%d\n", from->type);
                     from->Vo=from->DV+Vp;
                     from->Po=from->Vo*from->Io;
                     from->Vi[0]=from->DV+from->Vo;
                     //printf("from->Vi[0]:%g\n", from->Vi[0]);
                     //if (p==q-1) from->Vi[0]=Vi; // dirty hack to improve accuracy
                     //printf("from->Vi[0]:%g\n", from->Vi[0]);
                     //printf("from->DV:%g\n", from->DV);
                     //printf("from->Vo:%g\n", from->Vo);
                     from->Pi[0]=from->Vi[0]*from->Ii[0];
                     from->out=1;
                     Vp=from->Vi[0];
                     from=from->from[0];
                  }
#endif
               } // found RS so need voltage partitor
               //printf("Vi[%d]:%g V as input for load\n", i, nPtr[s].Vi[i]);
            } else { // no from, so no input connection
               //printf("nPtr[s].from[i] NULL\n");
               //nPtr[s].Vi[i]=0;
               continue;
            }
         }
         //printf("calc i:%d\n", i);
         if (nPtr[s].R[i]==0 && nPtr[s].Ii[i]!=0) { // know V,I ==> R,P
            nPtr[s].R[i]=calcR(nPtr[s].Vi[i], nPtr[s].Ii[i]);
            nPtr[s].Pi[i]=calcP(nPtr[s].Vi[i], nPtr[s].Ii[i]);
         }
         if (nPtr[s].Ii[i]==0 && nPtr[s].R[i]!=0) { // know V,R ==> I,P
            nPtr[s].Ii[i]=calcI(nPtr[s].Vi[i], nPtr[s].R[i]);
            nPtr[s].Pi[i]=calcP(nPtr[s].Vi[i], nPtr[s].R[i]);
         }
         nPtr[s].Pd+=nPtr[s].Pi[i]; // total dissipation

         // follow graph to root
         //printf("follow graph to root ...\n");
         //printf("s:%d\n", s);
         //printf("following input i:%d\n", i);
         //printf("name:'%s'\n", nPtr[s].name);
         from=nPtr[s].from[i];
         double Io=nPtr[s].Ii[i]; // pass current to node up
         while (from!=NULL) { // IN node
            //printf("\n");
            //printf("name:'%s'\n", from->name);
            //printf("type:%d\n", from->type);
            type=from->type;
            //printf("s:%d node:'%s' node:%p input:%d from:'%s' type:%d\n", s, nPtr[s].name, &nPtr[s], i, from->name, type);
            int ret;
            switch (type) {
            case 0: // IN
               //printf("case IN\n");
               //printf("IN i:%d from->Io:%g\n", i, from->Io);
               //printf("IN i:%d from->out:%d\n", i, from->out);
               //printf("IN i:%d from->to[0]:%p\n", i, from->to[0]);
               //printf("IN i:%d Io:%g\n", i, Io);
               ret=calcIN(from, Io);
               if (ret==1) goto skip; // stop graph exploration to root
               break;
            case 1: // SR
               //printf("case SR\n");
               //printf("SR i:%d node:%p name:%s from:%p\n", i, &nPtr[s], nPtr[s].name, from);
               //printf("SR i:%d from->name:%s\n", i, from->name);
               //printf("SR i:%d from->Io:%g\n", i, from->Io);
               //printf("SR i:%d from->out:%d\n", i, from->out);
               //printf("SR i:%d from->to[0]:%p\n", i, from->to[0]);
               //printf("SR i:%d from->to[0]->Ii[0]:%g\n", i, from->to[0]->Ii[0]);
               // discover what is the output that connect to current son
               //int o=0;
               //for (; o<MaxOut || from->to[o]==NULL; o++) {
               //   if (from->to[o]==&nPtr[s]) break;
               //}
               //printf("SR i:%d o:%d\n", i, o);
               //printf("SR i:%d Io:%g\n", i, Io);
               ret=calcSR(from, &Io);
               if (ret==1) goto skip; // stop graph exploration to root
               break;
            case 2: // LR
               //printf("LR i:%d from->Io:%g\n", i, from->Io);
               ret=calcLR(from, &Io);
               if (ret==1) goto skip; // stop graph exploration to root
               break;
            case 3: // LD
               //printf("case LD\n");
               printf("ERROR: usupported LD on root path\n");
               return -1;
               break;
            case 4: // RS
               //printf("case RS\n");
               printf("WARN: RS on root path not fully tested\n");
               if (from->out==0) { // RS not processed
                  //printf("RS processing ...\n");
                  ret=calcRS(from, Io, &Vo);
                  if (missFrom!=NULL) {
                     //printf("Processing late node...\n");
                     double I0=0;
                     if (missFrom->type==1) calcSR(missFrom, &I0);
                     if (missFrom->type==2) calcLR(missFrom, &I0);
                  }
               } else {
                  //printf("RS already processed, skip\n");
               }
               break;
            default:
               printf("ERROR: unsupported type:%d\n", type);
               return -1;
            } // switch (type)
            //printf("s:%d node:'%s'\n", s, nPtr[s].name);
            //printf("going up following node:'%s' input i:%d\n", nPtr[s].name, i);
            from=from->from[0];
         } // while (from!=NULL)
         skip: // stop graph exploration to root
         //printf("s:%d node:'%s' input:%d check next input\n", s, nPtr[s].name, i);
      } // for (int i=0; i<MaxIns; i++) // for every load input
      //printf("s:%d node:'%s' check next node\n", s, nPtr[s].name);
   } // for (int s=0; s<sect; s++) // INI sections = # nodes
   printf("done\n");
   printf("\n");
   return 0;
} // int calcNodes();

int showStructData() {
   // show struct data
   printf("show struct data\n");
   for (int s=0; s<sect; s++) { // INI sections = # nodes
      char nodeName[5];
      if (nPtr[s].type==-1) continue; // skip BOARD
      printf("node:%d\n", s);
      printf("ptr addr:%p\n", &nPtr[s]);
      strcpy(nodeName, nPtr[s].name);
      printf("node:'%s' key:type=%d\n", nodeName, nPtr[s].type);
      printf("node:'%s' key:label='%s'\n", nodeName, nPtr[s].label);
      printf("node:'%s' key:refdes='%s'\n", nodeName, nPtr[s].refdes);
      for (int i=0; i<MaxIns; i++) {
         if (nPtr[s].from[i]==NULL) continue; // here break is better
         printf("node:'%s' key:from[%d]=%p\n", nodeName, i, nPtr[s].from[i]);
         printf("node:'%s' key:Vi[%d]  =%g\n", nodeName, i, nPtr[s].Vi[i]);
         printf("node:'%s' key:Ii[%d]  =%g\n", nodeName, i, nPtr[s].Ii[i]);
         printf("node:'%s' key:R[%d]   =%g\n", nodeName, i, nPtr[s].R[i]);
         printf("node:'%s' key:Pi[%d]  =%g\n", nodeName, i, nPtr[s].Pi[i]);
      }
      printf("node:'%s' key:Iadj=%g\n", nodeName, nPtr[s].Iadj);
      printf("node:'%s' key:yeld=%g\n", nodeName, nPtr[s].yeld);
      printf("node:'%s' key:DV  =%g\n", nodeName, nPtr[s].DV);
      printf("node:'%s' key:Pd  =%g\n", nodeName, nPtr[s].Pd);
      printf("node:'%s' key:Vo  =%g\n", nodeName, nPtr[s].Vo);
      printf("node:'%s' key:Io  =%g\n", nodeName, nPtr[s].Io);
      printf("node:'%s' key:Po  =%g\n", nodeName, nPtr[s].Po);
      for (int t=0; t<MaxOut; t++) {
         if (nPtr[s].to[t]!=NULL) 
         printf("node:'%s' key:to[%d]  =%p\n", nodeName, t, nPtr[s].to[t]);
      }
      printf("node:'%s' key:out =%d\n", nodeName, nPtr[s].out);
      printf("\n");
   }
   return 0;
} // int showStructData()

int saveINIres(nTy* nPtr, int nodes) {
   char bufferPtr[5000]="";
   printf("Writing nodes:%d to INIres file:'%s'\n", nodes, DefCliIniResFile);
   int out=0;
   //out+=sprintf(bufferPtr+out, "[BOARD]\n");
   //out+=sprintf(bufferPtr+out, "label=%s\n", "ES3");
   //out+=sprintf(bufferPtr+out, "\n");
   for (int n=0; n<nodes; n++) {
      int type=nPtr[n].type;
      out+=sprintf(bufferPtr+out, "[%s]\n", nPtr[n].name);
      out+=sprintf(bufferPtr+out, "label=%s\n", nPtr[n].label);
      if (type!=-1 && type!=0) { // no BOARD and IN
         out+=sprintf(bufferPtr+out, "refdes=%s\n", nPtr[n].refdes);
      }
      if (type==3) { // LDx
         for (int i=0; i<MaxIns; i++) {
            if (nPtr[n].from[i]==NULL) continue; // LDx can have more than one
            out+=sprintf(bufferPtr+out, "f%d=%s\n", i, nPtr[n].in[0]);
            out+=sprintf(bufferPtr+out, "V%d=%g\n", i, nPtr[n].Vi[i]);
            out+=sprintf(bufferPtr+out, "I%d=%g\n", i, nPtr[n].Ii[i]);
            out+=sprintf(bufferPtr+out, "R%d=%g\n", i, nPtr[n].R[i]);
            out+=sprintf(bufferPtr+out, "P%d=%g\n", i, nPtr[n].Pi[i]);
         }
      } else if (type!=-1 && type!=0) { // no BOARD and IN and LDx
         out+=sprintf(bufferPtr+out, "f0=%s\n", nPtr[n].in[0]);
         out+=sprintf(bufferPtr+out, "Vi=%g\n", nPtr[n].Vi[0]);
         out+=sprintf(bufferPtr+out, "Ii=%g\n", nPtr[n].Ii[0]);
         out+=sprintf(bufferPtr+out, "Pi=%g\n", nPtr[n].Pi[0]);
      }
      if (type!=-1 && type!=0) { // no BOARD and IN
         if (type!=3) { // no LOAD
            out+=sprintf(bufferPtr+out, "DV=%g\n", nPtr[n].DV);
            if (!strncasecmp(nPtr[n].name, "SR", 2)) {
               out+=sprintf(bufferPtr+out, "n=%g\n", nPtr[n].yeld);
            } else {
               out+=sprintf(bufferPtr+out, "Iadj=%g\n", nPtr[n].Iadj);
            }
         }
         out+=sprintf(bufferPtr+out, "Pd=%g\n", nPtr[n].Pd);
      }
      if (type==0) { // IN
         out+=sprintf(bufferPtr+out, "V=%g\n", nPtr[n].Vo);
         out+=sprintf(bufferPtr+out, "I=%g\n", nPtr[n].Io);
         out+=sprintf(bufferPtr+out, "P=%g\n", nPtr[n].Po);
      } else if (type!=-1 && type!=3) { // LDx
         out+=sprintf(bufferPtr+out, "Vo=%g\n", nPtr[n].Vo);
         out+=sprintf(bufferPtr+out, "Io=%g\n", nPtr[n].Io);
         out+=sprintf(bufferPtr+out, "Po=%g\n", nPtr[n].Po);
      }
      out+=sprintf(bufferPtr+out, "\n");
      //printf("buffer[%d]:'\n%s\n'\n", i, bufferPtr);
   }
   //printf("\n");
   //printf("buffer:'\n%s\n'\n", bufferPtr);
   //int len=sizeof(bufferPtr);
   //printf("out:%d len:%d\n", out, len);
   FILE* filePtr=openWrite(DefCliIniResFile);
   fwrite(bufferPtr, 1, out, filePtr);
   fclose(filePtr);
   printf("Written out:%d Bytes\n", out);
   return 0;
} // int saveINIres(nTy* nPtr, int nodes)

int freeMem() {
   //printf("freeMem nPtr:%p\n", nPtr);
   free(nPtr);
   iniparser_freedict(graphPtr);
   return 0;
} // int freeMem()
