/* PowerBudget v0.00.01a 2024/09/08 calculate power dissipation and budget */
/* Copyright 2024 Valerio Messina http://users.iol.it/efa              */
/* powerbGui.c is part of PowerBudget
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

/* powerbGui.c GUI main: read INI(res) file, create/modify node graph, insert/modify values, write INI file */

/* nuklear - 1.32.0 - public domain */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <SDL2/SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
//#define NK_STRTOD strtod
//#define NK_DTOA dtoa
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

#define WINDOW_WIDTH  1860
#define WINDOW_HEIGHT 1010

#include "powerbLib.h"
#include "fileIo.h"

#define PRINTOFF      0
#define PRINTERROR    1
#define PRINTWARN     2
#define PRINTBATCH    3
#define PRINTF        4
#define PRINTDEBUG    5
#define PRINTVERBOSE  6
#define PRINTALL      7

u08 dbgLev=PRINTF;

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
#define INCLUDE_NODE_EDITOR

#ifdef INCLUDE_NODE_EDITOR
  #include "node_editor.c"
#endif

//extern struct node_editor nodeEditor;

//nTy* nPtr=NULL;
//int nCnt=0;

int initNodeData(nTy* nodePtr) { // init to empty a nTy node
   int i;
   nodePtr->name[0]='\0';
   nodePtr->type=0; // IN=0, SR=1, LR=2, RS=4, LD=3
   nodePtr->label[0]='\0'; // any user string
   nodePtr->refdes[0]='\0'; // "Uxx" or "RNxxxx"
   nodePtr->col=-1; // used for positioning
   nodePtr->row=-1; // used for positioning
   for (i=0; i<MaxIns; i++) {
      nodePtr->from[i]=NULL; // SR,LR,RS has 1, LD up to MaxIns
      nodePtr-> in[i][0]='\0'; // used only by GUI
      nodePtr->Vi[i]=0;
      nodePtr->Ii[i]=0;
      nodePtr->R[i]=0;
      nodePtr->Pi[i]=0;
   }
   nodePtr->yeld=0;
   nodePtr->Iadj=0;
   nodePtr->DV=0;
   nodePtr->Pd=0;
   nodePtr->Vo=0;
   nodePtr->Io=0;
   nodePtr->Po=0;
   for (i=0; i<MaxOut; i++) {
      nodePtr->to[i]=NULL; // to leaf
   }
   nodePtr->out=0;
   return 0;
} // int initNodeData(nTy* nodePtr)


int fillNodeData(int id, nTy* valuesPtr) {
   struct node* nodePtr;
   nodePtr=node_editor_find(&nodeEditor, id);
   //nCnt++;
   //size_t size=sizeof(nTy);
   //printf("id:%d nodePtr:%p nCnt:%d size:%d\n", id, nodePtr, nCnt, size);
   //printf("id:%d nodePtr:%p size:%d\n", id, nodePtr, size);
   //printf("nPtr:%p\n", nPtr);
   //nPtr=realloc(nPtr, sizeof(nCnt*size)); // allocation space for nodes
   //printf("nPtr:%p\n", nPtr);
   //void* mPtr=NULL;
   //printf("valuesPtr->Vo:%g\n", valuesPtr->Vo);
   //mPtr=memcpy(&nPtr[nCnt-1], valuesPtr, size); // copy vector element
   //printf("coping node to node_editor struct ...\n");
   //nTy* valuesNewPtr=malloc(sizeof(nTy)); // allocate space for an nTy
   //*valuesNewPtr=*valuesPtr; // copy whole nTy
   nodePtr->valuesPtr=valuesPtr; // link GUI node to values node
   //printf("node:%p valuesPtr:%p\n", nodePtr, nodePtr->valuesPtr);
   //printf("mPtr:%p\n", mPtr);
   //printf("coping address to node_editor struct ...\n");
   //nodePtr->values=nPtr[nCnt-1]; // so node_editor struct point to allocated vector
   return 0;
} // int fillNodeData(int id, nTy* valuesPtr)

int saveINI(void* nodedit) {
   struct node_editor* nodeditPtr;
   nodeditPtr=nodedit;
   struct node* nodePtr;
   char bufferPtr[5000]="";
   int out=0;
   out+=sprintf(bufferPtr+out, "[BOARD]\n");
   out+=sprintf(bufferPtr+out, "label=%s\n", "ES3");
   out+=sprintf(bufferPtr+out, "\n");
   nodePtr=nodeditPtr->begin;
   for (int n=0; n<nodeditPtr->node_count; n++) {
      int type=nodePtr->valuesPtr->type;
      //printf("name:'%s' type:%d add:%p\n", nodePtr->valuesPtr->name, type, nodePtr);
      out+=sprintf(bufferPtr+out, "[%s]\n", nodePtr->valuesPtr->name);
      out+=sprintf(bufferPtr+out, "label=%s\n", nodePtr->valuesPtr->label);
      if (type!=0) { // not for IN
         out+=sprintf(bufferPtr+out, "refdes=%s\n", nodePtr->valuesPtr->refdes);
      }
      if (type==3) { // LDx
         for (int i=0; i<MaxIns; i++) {
            //printf("n:%d i:%d\n", n, i);
            if (nodePtr->valuesPtr->in[i][0]=='\0') continue; // LDx can have more than one
            out+=sprintf(bufferPtr+out, "f%d=%s\n", i, nodePtr->valuesPtr->in[i]);
            out+=sprintf(bufferPtr+out, "V%d=%g\n", i, nodePtr->valuesPtr->Vi[i]);
            out+=sprintf(bufferPtr+out, "I%d=%g\n", i, nodePtr->valuesPtr->Ii[i]);
            out+=sprintf(bufferPtr+out, "R%d=%g\n", i, nodePtr->valuesPtr->R[i]);
            out+=sprintf(bufferPtr+out, "P%d=%g\n", i, nodePtr->valuesPtr->Pi[i]);
         }
      } else if (type!=-1 && type!=0) { // no BOARD and IN and LDx
         out+=sprintf(bufferPtr+out, "f0=%s\n", nodePtr->valuesPtr->in[0]);
         out+=sprintf(bufferPtr+out, "Vi=%g\n", nodePtr->valuesPtr->Vi[0]);
         out+=sprintf(bufferPtr+out, "Ii=%g\n", nodePtr->valuesPtr->Ii[0]);
         out+=sprintf(bufferPtr+out, "Pi=%g\n", nodePtr->valuesPtr->Pi[0]);
      }
      if (type!=0) { // IN
         out+=sprintf(bufferPtr+out, "DV=%g\n", nodePtr->valuesPtr->DV);
         out+=sprintf(bufferPtr+out, "n=%g\n", nodePtr->valuesPtr->yeld);
         out+=sprintf(bufferPtr+out, "Iadj=%g\n", nodePtr->valuesPtr->Iadj);
         out+=sprintf(bufferPtr+out, "Pd=%g\n", nodePtr->valuesPtr->Pd);
      }
      if (type==0) { // IN
         out+=sprintf(bufferPtr+out, "V=%g\n", nodePtr->valuesPtr->Vo);
         out+=sprintf(bufferPtr+out, "I=%g\n", nodePtr->valuesPtr->Io);
         out+=sprintf(bufferPtr+out, "P=%g\n", nodePtr->valuesPtr->Po);
      } else if (type!=3) { // LDx
         out+=sprintf(bufferPtr+out, "Vo=%g\n", nodePtr->valuesPtr->Vo);
         out+=sprintf(bufferPtr+out, "Io=%g\n", nodePtr->valuesPtr->Io);
         out+=sprintf(bufferPtr+out, "Po=%g\n", nodePtr->valuesPtr->Po);
      }
      out+=sprintf(bufferPtr+out, "\n");
      //printf("buffer[%d]:'\n%s\n'\n", n, bufferPtr);
      nodePtr=nodePtr->next;
   }
   //printf("\n");
   //printf("buffer:'\n%s\n'\n", bufferPtr);
   //int len=sizeof(bufferPtr);
   //printf("out:%d len:%d\n", out, len);
   FILE* filePtr=openWrite(DefGuiIniFile);
   fwrite(bufferPtr, 1, out, filePtr);
   fclose(filePtr);
   printf("Written out:%d Bytes file:'%s'\n", out, DefGuiIniFile);
   return 0;
} // int saveINI(void* nodedit)

int calcINI() {
   system("powerb "DefGuiIniFile);
   return 0;
} // int calcINI()

// find the GUI node that point to values nodePtr
int findGuiNode(void* nodedit, nTy* nodeValPtr) {
   struct node_editor* nodeditPtr;
   nodeditPtr=nodedit;
   struct node* nodePtr;
   //printf("graph searching  node:%p\n", nodeValPtr);
   nodePtr=nodeditPtr->begin;
   for (int n=0; n<nodeEditor.node_count; n++) {
      //printf("graph n:%02d id:%02d node:%p valuesPtr:%p\n", n, nodePtr->ID, nodePtr, nodePtr->valuesPtr);
      //printf("graph n:%02d id:%02d name:'%s' values.to.name:'%s'\n", n, nodePtr->ID, nodePtr->name, nodePtr->valuesPtr->to[0]->name);
      if (nodePtr->valuesPtr->type==-1) continue; // BOARD
      if (nodePtr->valuesPtr==nodeValPtr) break;
      nodePtr=nodePtr->next;
   }
   return nodePtr->ID;
} // int findGuiNode(void* nodedit, nTy* nodeValPtr)

int loadINIres(void* nodedit) {
   struct node_editor* nodeditPtr;
   nodeditPtr=nodedit;
   struct node* nodePtr;
   // at first remove all existing nodes and links
   printf("removing current nodes ...\n");
   int pass=0;
   int found;
   do {
      found=0;
      nodePtr=nodeditPtr->begin;
      if (nodePtr) {
         //printf("Remove node:%p\n", nodePtr);
         node_editor_delnode(nodeditPtr, nodePtr);
         found=1;
         //nodePtr=nodePtr->next;
      }
      pass++;
   } while (found); // removed all nodes
   //printf("found:%d pass:%d\n", found, pass);
   freeMem(); // free values
   printf("cleared\n");
   //printf("\n");

   printf("init ...\n");
   node_editor_init(&nodeEditor);
   nodeEditor.initialized = 1;
   int id;
   //nTy node;
   //char name[5];

   //int ret;
   int sect;
   printf("loading ...\n");
   loadINI(DefCliIniResFile, &sect);
   printf("loaded %d sections, %d nodes\n", sect, sect-1);
   printf("\n");
   //showStructData();

   // create GUI nodes
   printf("Creating GUI nodes ...\n");
   for (int n=0; n<sect; n++) {
      //printf("graph n:%02d node:'%s'\n", n, nPtr[n].name);
      //if (!strcasecmp(nPtr[n].name, "board")) continue;
      if (nPtr[n].type==-1) continue; // BOARD
      int in=1, out=1;
      if (!strcasecmp(nPtr[n].name, "in")) in=0;
      if (!strncasecmp(nPtr[n].name, "ld", 2)) out=0;
      id=node_editor_add(nodeditPtr, nPtr[n].name, nk_rect(OFFSET+(3-nPtr[n].col)*(NODE_WIDTH+SPACING), OFFSET+nPtr[n].row/3*(NODE_HEIGHT+SPACING), NODE_WIDTH, NODE_HEIGHT), nk_rgb(255,   0,  0), in, out);
      //printf("created GUI node:%d\n", id);
      //printf("values addr node:%p\n", &nPtr[n]);
      fillNodeData(id, &nPtr[n]);
   }

//#if 0
   // create GUI links
   printf("Creating GUI links ...\n");
   nodePtr=nodeditPtr->begin;
   for (int n=0; n<nodeEditor.node_count; n++) {
      //printf("graph n:%02d id:%02d addr:%p\n", n, nodePtr->ID, nodePtr);
      if (nodePtr->valuesPtr->type==-1) continue; // BOARD
      if (nodePtr->valuesPtr->type==3) continue; // LD
      for (int l=0; l<MaxOut; l++) {
         if (nodePtr->valuesPtr->to[l]!=NULL) {
            //printf("grap_ n:%02d l:%02d node:%p valuesPtr:%p\n", n, l, nodePtr, nodePtr->valuesPtr);
            //printf("grap_ n:%02d l:%02d name:'%s' values.to.name:'%s' addr:%p\n", n, l, nodePtr->name, nodePtr->valuesPtr->to[l]->name, nodePtr->valuesPtr->to[l]);
            int id=findGuiNode(nodeditPtr, nodePtr->valuesPtr->to[l]);
            node_editor_link(nodeditPtr, nodePtr->ID, 0, id, 0); // link two nodes: nodeIDfrom, slotIDfrom, nodeIDto, slotIDto
         }
      }
      nodePtr=nodePtr->next;
   }
//#endif
   return 0;
} // int loadINIres(void* nodedit)

#if 0
char* dtoa(double d) { // convert a double to an auto-allocated string. Remember to free the string
   char** strPtrPtr;
   asprintf(strPtrPtr, "%g", d);
   return *strPtrPtr;
}

char* mystrcat(char* dest, char* src) {
   while (*dest) dest++;
   while (*dest++ = *src++);
   return --dest;
}
#endif

const nk_rune nk_font_mathematical_glyph_ranges[] = {
  0x0020, 0x00FF,
  0x0370, 0x03FF, // Greek and Coptic
  0x2200, 0x22FF, // Mathematical Operators
  0
};

#if 0
NK_API const nk_rune*
nk_font_mathematical_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x0370, 0x03FF, // Greek and Coptic
        0x2200, 0x22FF, // Mathematical Operators
        0
    };
    return ranges;
}
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int
main(int argc, char *argv[])
{
    /* Platform */
    SDL_Window *win;
    SDL_Renderer *renderer;
    int running = 1;
    int flags = 0;
    float font_scale = 1;

    /* GUI */
    struct nk_context *ctx;
    struct nk_colorf bg;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("PowerbGui",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    if (win == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        exit(-1);
    }

    flags |= SDL_RENDERER_ACCELERATED;
    flags |= SDL_RENDERER_PRESENTVSYNC;

#if 0
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#endif

    renderer = SDL_CreateRenderer(win, -1, flags);

    if (renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        exit(-1);
    }

    /* scale the renderer output for High-DPI displays */
    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
        SDL_GetWindowSize(win, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_RenderSetScale(renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    /* GUI */
    ctx = nk_sdl_init(win, renderer);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        config.range = &nk_font_mathematical_glyph_ranges[0];
        nk_sdl_font_stash_begin(&atlas);
        //nk_font_atlas_init_default(atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    while (running)
    {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            nk_sdl_handle_event(&evt);
        }
        nk_sdl_handle_grab(); /* optional grabbing behavior */
        nk_input_end(ctx);

        /* GUI */

        /* -------------- EXAMPLES ---------------- */
        #ifdef INCLUDE_NODE_EDITOR
          node_editor(ctx);
        #endif
        /* ----------------------------------------- */

        SDL_SetRenderDrawColor(renderer, bg.r * 255, bg.g * 255, bg.b * 255, bg.a * 255);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
    }

cleanup:
    freeMem();
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
} // int main(int argc, char *argv[])
