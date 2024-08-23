/* PowerBudget v0.00.01a 2024/08/20 calculate power dissipation and budget */
/* Copyright 2024 Valerio Messina http://users.iol.it/efa              */
/* powerb.c is part of PowerBudget
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

/* powerb.c CLI main: read INI file, calc engine, write INIres file */

#include <stdio.h>
#include <strings.h>

#include "powerbLib.h"
#include "fileIo.h"

u08 dbgLev=PRINTF;

int main(int argNum, char* argV[]) {
   int ret;
   //printf("argNum:%d\n", argNum);
   if (argNum>0) argNum--;
   //printf("argNum:%d\n", argNum);
   if (argNum>1) {
      printf("WARN: ignoring argNum >1\n");
   }
   //if (argNum>0) printf("INI file argV:'%s'\n", argV[1]);
   // choose ini file
   char* graphFile;
   if (argNum>0) {
      int len=strlen(argV[1]);
      graphFile=malloc(len);
      strcat(graphFile, argV[1]);
   } else { // default fileName
      char fileName[]=DefIniFile;
      int len=strlen(fileName);
      graphFile=malloc(len);
      strcat(graphFile, fileName);
   }
   printf("INI file:'%s'\n", graphFile);

   int sect;
   ret=loadINI(graphFile, &sect);

   ret=calcNodes();

   //ret=showStructData();

   //printf("Tot Sect:%d Nodes:%d\n", sect, nt);
   saveINIres(nPtr, sect);

   ret=freeMem();
   return 0;
}
