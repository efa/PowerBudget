/* PowerBudget v0.00.01a 2024/08/20 calculate power dissipation and budget */
/* Copyright 2024 Valerio Messina http://users.iol.it/efa              */
/* node_editor.c is part of PowerBudget
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

/* node_editor.c GUI: node_editor */

/* nuklear - v1.00 - public domain */
/* This is a simple node editor just to show a simple implementation and that
 * it is possible to achieve it with this library. While all nodes inside this
 * example use a simple color modifier as content you could change them
 * to have your custom content depending on the node time.
 * Biggest difference to most usual implementation is that this example does
 * not have connectors on the right position of the property that it links.
 * This is mainly done out of laziness and could be implemented as well but
 * requires calculating the position of all rows and add connectors.
 * In addition adding and removing nodes is quite limited at the
 * moment since it is based on a simple fixed array. If this is to be converted
 * into something more serious it is probably best to extend it.*/

#include <limits.h>
#include <float.h>
#include "powerbLib.h"

//#define NODE_WIDTH  256
#define NODE_WIDTH  316
#define NODE_HEIGHT 130
#define OFFSET       20
#define SPACING      75

struct node {
    int ID;
    char name[32];
    struct nk_rect bounds;
    struct nk_color color;
    int input_count;
    int output_count;
    struct node *prev;
    struct node *next;
    nTy values;
};

struct node_link {
    int input_id;
    int input_slot;
    int output_id;
    int output_slot;
    struct nk_vec2 in;
    struct nk_vec2 out;
    struct node_link* prev;
    struct node_link* next;
};

struct node_linking {
    int active;
    struct node *node;
    int input_id;
    int input_slot;
};

struct node_editor {
    int initialized;
    struct node node_buf[32];
    struct node_link links[64];
    struct node *begin;
    struct node *end;
    struct node_link* firstlink;
    struct node_link* lastlink;
    int node_count;
    int link_count;
    struct nk_rect bounds;
    struct node *selected;
    int show_grid;
    struct nk_vec2 scrolling;
    struct node_linking linking;
};
struct node_editor nodeEditor;


// add a node to linked list as last element
static void
node_editor_push(struct node_editor *editor, struct node *node)
{
    if (!editor->begin) { // first node
        node->next = NULL;
        node->prev = NULL;
        editor->begin = node;
        editor->end = node;
    } else { // next nodes
        node->prev = editor->end;
        if (editor->end) // can remove condition
            editor->end->next = node;
        node->next = NULL;
        editor->end = node;
    }
}

int nodeShow(struct node* nodePtr) {
   if (nodePtr==NULL) return 1;
   printf("node ID:%d\n", nodePtr->ID);
   printf("node name:'%s'\n", nodePtr->name);
   //printf("node struct nk_rect bounds\n", nodePtr->);
   //printf("node struct nk_color color\n", nodePtr->);
   printf("node input_count:%d\n", nodePtr->input_count);
   printf("node output_count:%d\n", nodePtr->output_count);
   printf("node prevPtr:%p\n", nodePtr->prev);
   printf("node nextPtr:%p\n", nodePtr->next);
   //printf("node values\n", nodePtr->prevPtr);
   printf("\n");
   return 0;
}

void nodeEditorShow() {
   printf("nodeEditor list node_count:%d\n", nodeEditor.node_count);
   struct node* nodePtr=nodeEditor.begin;
   for (int n=0; n<nodeEditor.node_count; n++) {
      printf("nodeEditor n:%d id:%d node:%p\n", n, nodePtr->ID, nodePtr);
      nodeShow(nodePtr);
      nodePtr=nodePtr->next;
   }
   printf("\n");
}

#if 0
void nodeEditorShowVect() {
  printf("nodeEditor list node_count:%d\n", nodeEditor.node_count);
  for (int n=0; n<nodeEditor.node_count; n++) {
      printf("nodeEditor n:%d id:%d node:%p\n", n, nodeEditor.node_buf[n].ID, &nodeEditor.node_buf[n]);
      nodeShow(&nodeEditor.node_buf[n]);
   }
   printf("\n");
}
#endif

int IDs = 0;

// inc editor->node_count, fill local node with data, then call node_push()
static int
node_editor_add(struct node_editor *editor, const char *name, struct nk_rect bounds,
    struct nk_color col, int in_count, int out_count)
{
    struct node *node;
    //NK_ASSERT((nk_size)editor->node_count < NK_LEN(editor->node_buf));
    printf("add node id:%d\n", IDs);
    //printf("editor->node_count:%d\n", editor->node_count);
//nodeEditorShow();
    //node = &editor->node_buf[editor->node_count++];
    node = malloc(sizeof(struct node));
    editor->node_count++;
    //printf("&node_buf[%d]:%p\n", editor->node_count-1, node);
    node->ID = IDs;
    strcpy(node->name, name);
    //sprintf(node->name, "%d", node->ID);
    node->bounds = bounds;
    node->color = nk_rgb(255, 0, 0);
    node->color = col;
    node->input_count = in_count;
    node->output_count = out_count;
    //node->value = 0;
    //node->values=NULL; // should init values too
    node_editor_push(editor, node);
    printf("id:%d node:%p\n", node->ID, node);
    printf("editorPtr->node_buf[%d] node_count:%d\n", node->ID, editor->node_count);
    IDs++;
    //printf("next IDs:%d\n", IDs);
    printf("\n");
//nodeEditorShow();
    return node->ID;
}

int linkShow(struct node_link* linkPtr) {
   if (linkPtr==NULL) return 1;
   printf("link input_id:%d\n", linkPtr->input_id);
   printf("link input_slot:%d\n", linkPtr->input_slot);
   printf("link output_id:%d\n", linkPtr->output_id);
   printf("link output_slot:%d\n", linkPtr->output_slot);
   printf("link prevPtr:%p\n", linkPtr->prev);
   printf("link nextPtr:%p\n", linkPtr->next);
   printf("\n");
   return 0;
}

void nodeEditorLinkShow() {
   printf("nodeEditor list links link_count:%d\n", nodeEditor.link_count);
   struct node_link* linkPtr=nodeEditor.firstlink;
   for (int n=0; n<nodeEditor.link_count; n++) {
      printf("nodeEditorLink n:%d link:%p\n", n, linkPtr);
      linkShow(linkPtr);
      linkPtr=linkPtr->next;
   }
   printf("\n");
}

// add a link to linked list as last element
void link_push(struct node_editor* editorPtr, struct node_link* linkPtr) {
    if (!editorPtr->firstlink) { // first link
        linkPtr->next = NULL;
        linkPtr->prev = NULL;
        editorPtr->firstlink = linkPtr;
        editorPtr->lastlink = linkPtr;
    } else { // next links
        linkPtr->prev = editorPtr->lastlink;
        if (editorPtr->lastlink) // can remove condition
            editorPtr->lastlink->next = linkPtr;
        linkPtr->next = NULL;
        editorPtr->lastlink = linkPtr;
    }
} // void link_push(struct node_editor* editorPtr, struct node_link* linkPtr)

// link two nodes
static void
node_editor_link(struct node_editor *editor, int in_id, int in_slot,
    int out_id, int out_slot)
{
    struct node_link *link;
    //NK_ASSERT((nk_size)editor->link_count < NK_LEN(editor->links));
    //link = &editor->links[editor->link_count++];
    link = malloc(sizeof(struct node_link));
    printf("add link:%p\n", link);
    printf("inid:%d in:%d outid:%d out:%d\n", in_id, in_slot, out_id, out_slot);
//nodeEditorLinkShow();
    editor->link_count++;
    link->input_id = in_id;
    link->input_slot = in_slot;
    link->output_id = out_id;
    link->output_slot = out_slot;
    link_push(editor, link);
    printf("\n");
//nodeEditorLinkShow();
}

// remove a node from the linked list
static void
node_editor_pop(struct node_editor *editor, struct node *node)
{
    if (node->next) // not last
        node->next->prev = node->prev; // can be NULL if first
    if (node->prev) // not first
        node->prev->next = node->next; // can be NULL if last
    if (editor->end == node) // last
        editor->end = node->prev;
    if (editor->begin == node) // first
        editor->begin = node->next;
    node->next = NULL; // just in case
    node->prev = NULL; // just in case
}

// remove a link from the linked list
void link_pop(struct node_editor* editorPtr, struct node_link* linkPtr) {
    if (linkPtr->next) // not last
        linkPtr->next->prev = linkPtr->prev; // can be NULL if first
    if (linkPtr->prev) // not first
        linkPtr->prev->next = linkPtr->next; // can be NULL if last
    if (editorPtr->lastlink == linkPtr) // last
        editorPtr->lastlink = linkPtr->prev;
    if (editorPtr->firstlink == linkPtr) // first
        editorPtr->firstlink = linkPtr->next;
    linkPtr->next = NULL; // just in case
    linkPtr->prev = NULL; // just in case
}

void node_editor_unlink(struct node_editor* editorPtr, struct node_link* linkPtr) {
    struct node_link* linkPrevPtr;
   //nodeEditorLinkShow();
   /*printf("link_count:%d\n", editorPtr->link_count);*/
   if (editorPtr->link_count>0) {
      link_pop(editorPtr, linkPtr);
      free(linkPtr);
      editorPtr->link_count--;
   }
   //nodeEditorLinkShow();
}

// remove all links going to or from a nome
void node_unlink(struct node_editor* editorPtr, struct node* nodePtr) {
   //nodeEditorLinkShow();
   struct node_link* linkPtr;
   int pass=0;
   int found;
   do {
      found=0;
      linkPtr=editorPtr->firstlink;
      while (linkPtr) {
         if(linkPtr->output_id==nodePtr->ID ||
            linkPtr->input_id==nodePtr->ID) {
            printf("Remove link:%p\n", linkPtr);
            node_editor_unlink(editorPtr, linkPtr);
            found=1;
            break;
         }
         linkPtr=linkPtr->next;
      }
      pass++;
   } while (found); // checked all links without find anyone connected to node
   printf("found:%d pass:%d\n", found, pass);
   printf("link_count:%d\n", editorPtr->link_count);
   //nodeEditorLinkShow();
}

static void
node_editor_delnode(struct node_editor* editorPtr, struct node* nodePtr) {
   if (editorPtr->node_count>0) {
      printf("delete node:%p\n", nodePtr);
      //nodeEditorShow();
      // at first remove all linking and linked links to the node
      node_unlink(editorPtr, nodePtr);
      // remove from the linked list
      node_editor_pop(editorPtr, nodePtr);
      free(nodePtr);
#if 0
      int p; // search node position in node_buf[p]
      for (p=0; p<editorPtr->node_count-1; p++) {
         if (&editorPtr->node_buf[p]==nodePtr) break;
      }
      printf("editorPtr->node_buf[%d] node_count:%d\n", p, editorPtr->node_count);
      // copy nodes back by one
      int n;
      for (n=p; n<editorPtr->node_count-1; n++) {
         printf("n:%d\n", n);
         printf("n:%d id:%d\n", n, editorPtr->node_buf[n].ID);
         memcpy(&editorPtr->node_buf[n],&editorPtr->node_buf[n+1],sizeof(nodePtr));
         printf("n:%d id:%d\n", n, editorPtr->node_buf[n].ID);
         editorPtr->node_buf[n].ID--;
         printf("n:%d id:%d\n", n, editorPtr->node_buf[n].ID);
      }
      // zero the node in node_buf[n] just in case
      printf("zero node_buf[%d]\n", n);
      memset(&editorPtr->node_buf[n], 0, sizeof(*nodePtr));
#endif
      editorPtr->node_count--;
      //IDs--;
      //printf("next IDs:%d\n", IDs);
      //nodeEditorShow();
   }
   return;
}

static struct node*
node_editor_find(struct node_editor *editor, int ID)
{
    struct node *iter = editor->begin;
    while (iter) {
        if (iter->ID == ID)
            return iter;
        iter = iter->next;
    }
    return NULL;
}

static void
node_editor_del(struct node_editor* editorPtr, int id) {
   int l;
   struct node* nodePtr;
   if (editorPtr->node_count>0) {
      nodePtr=node_editor_find(editorPtr, id);
      printf("del id:%d node:%p\n", id, nodePtr);
      node_editor_delnode(editorPtr, nodePtr);
   }
   return;
}

static void
node_editor_init(struct node_editor *editor)
{
    memset(editor, 0, sizeof(*editor));
    editor->begin = NULL;
    editor->end = NULL;
    editor->firstlink = NULL;
    editor->lastlink = NULL;
    editor->node_count=0;
    editor->link_count=0;
    editor->show_grid = nk_true;

    int id;
    nTy node;
    char name[5];

    strcpy(name, "IN");
    id=node_editor_add(editor, name, nk_rect(OFFSET                       , OFFSET                        , NODE_WIDTH, NODE_HEIGHT), nk_rgb(255,   0,  0), 0, 1);
    initNodeData(&node);
    strcpy(node.name, name); node.type=0; strcpy(node.label, name); node.Vo=5;
    fillNodeData(id, &node);

//#if 0
    strcpy(name, "SR1");
    id=node_editor_add(editor, name, nk_rect(OFFSET+1*(NODE_WIDTH+SPACING), OFFSET                        , NODE_WIDTH, NODE_HEIGHT), nk_rgb(  0, 255,  0), 1, 1);
    initNodeData(&node);
    strcpy(node.name, name); node.type=1; strcpy(node.label,"Buck"); strcpy(node.refdes,"U14");
    strcpy(node.in[0],"IN"); node.yeld=0.9; node.Vo=1.8;
    fillNodeData(id, &node);
    strcpy(name, "LR1");
    id=node_editor_add(editor, name, nk_rect(OFFSET+1*(NODE_WIDTH+SPACING), OFFSET+1*(NODE_HEIGHT+SPACING), NODE_WIDTH, NODE_HEIGHT), nk_rgb(  0,   0,255), 1, 1);
    initNodeData(&node);
    strcpy(node.name, name); node.type=2; strcpy(node.label,"LDO1"); strcpy(node.refdes,"U12");
    strcpy(node.in[0],"IN"); node.Iadj=0.005; node.Vo=3.6;
    fillNodeData(id, &node);
    strcpy(name, "LR2");
    id=node_editor_add(editor, name, nk_rect(OFFSET+2*(NODE_WIDTH+SPACING), OFFSET+1*(NODE_HEIGHT+SPACING), NODE_WIDTH, NODE_HEIGHT), nk_rgb(  0,   0,255), 1, 1);
    initNodeData(&node);
    strcpy(node.name, name); node.type=2; strcpy(node.label,"LDO2"); strcpy(node.refdes,"U13");
    strcpy(node.in[0],"LR1"); node.Iadj=0.005; node.Vo=3.3L;
    fillNodeData(id, &node);
    strcpy(name, "LD1");
    id=node_editor_add(editor, name, nk_rect(OFFSET+3*(NODE_WIDTH+SPACING), OFFSET                        , NODE_WIDTH, NODE_HEIGHT), nk_rgb(255, 255,  0), 3, 0);
    initNodeData(&node);
    strcpy(node.name, name); node.type=3; strcpy(node.label,"SpW"); strcpy(node.refdes,"U20");
    strcpy(node.in[0],"SR1"); node.Ii[0]=0.528; strcpy(node.in[1],"SR1"); node.Ii[1]=0.008; strcpy(node.in[2],"LR2"); node.Ii[2]=0.317;
    fillNodeData(id, &node);
    strcpy(name, "LD2");
    id=node_editor_add(editor, name, nk_rect(OFFSET+3*(NODE_WIDTH+SPACING), OFFSET+1*(NODE_HEIGHT+SPACING), NODE_WIDTH, NODE_HEIGHT), nk_rgb(255, 255,  0), 1, 0);
    initNodeData(&node);
    strcpy(node.name, name); node.type=3; strcpy(node.label,"LVDS"); strcpy(node.refdes,"U6");
    strcpy(node.in[0],"LR2"); node.Ii[0]=0.0354;
    fillNodeData(id, &node);
    node_editor_link(editor, 0, 0, 1, 0);
    node_editor_link(editor, 0, 0, 2, 0);
    node_editor_link(editor, 2, 0, 3, 0);
    node_editor_link(editor, 1, 0, 4, 0);
    node_editor_link(editor, 1, 0, 4, 1);
    node_editor_link(editor, 3, 0, 4, 2);
    node_editor_link(editor, 3, 0, 5, 0);
//#endif
}

int nodeclick=0;
int nodeid=0;
//char text[10]="text";

static int
node_editor(struct nk_context *ctx)
{
    int n = 0;
    struct nk_rect total_space;
    const struct nk_input *in = &ctx->input;
    struct nk_command_buffer *canvas;
    struct node *updated = 0;
    struct node_editor *nodedit = &nodeEditor;

    if (!nodeEditor.initialized) {
        node_editor_init(&nodeEditor);
        nodeEditor.initialized = 1;
    }

    if (nk_begin(ctx, "PowerBudget", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_CLOSABLE))
    {
        /* allocate complete window space */
        canvas = nk_window_get_canvas(ctx);
        total_space = nk_window_get_content_region(ctx);
        nk_layout_space_begin(ctx, NK_STATIC, total_space.h, nodedit->node_count);
        {
            int inclick=0;
            int id;
            int inp;
            struct node_link* linkSavePtr;
            struct nk_rect size = nk_layout_space_bounds(ctx);
            struct nk_panel *node = 0;

            if (nodedit->show_grid) {
                /* display grid */
                float x, y;
                const float grid_size = 32.0f;
                const struct nk_color grid_color = nk_rgb(50, 50, 50);
                for (x = (float)fmod(size.x - nodedit->scrolling.x, grid_size); x < size.w; x += grid_size)
                    nk_stroke_line(canvas, x+size.x, size.y, x+size.x, size.y+size.h, 1.0f, grid_color);
                for (y = (float)fmod(size.y - nodedit->scrolling.y, grid_size); y < size.h; y += grid_size)
                    nk_stroke_line(canvas, size.x, y+size.y, size.x+size.w, y+size.y, 1.0f, grid_color);
            }

            /* execute each node as a movable group */
            struct node *it = nodedit->begin;
            while (it) {
                /* calculate scrolled node window position and size */
                nk_layout_space_push(ctx, nk_rect(it->bounds.x - nodedit->scrolling.x,
                    it->bounds.y - nodedit->scrolling.y, it->bounds.w, it->bounds.h));

                /* execute node window */
                if (nk_group_begin(ctx, it->name, NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER|NK_WINDOW_TITLE))
                {
                    /* always have last selected node on top */

                    node = nk_window_get_panel(ctx);
                    if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, node->bounds) &&
                        (!(it->prev && nk_input_mouse_clicked(in, NK_BUTTON_LEFT,
                        nk_layout_space_rect_to_screen(ctx, node->bounds)))) &&
                        nodedit->end != it)
                    {
                        updated = it;
                    }

                    if (nk_input_mouse_clicked(in, NK_BUTTON_RIGHT, node->bounds)) {
                        nodeclick=1;
                        /*printf("right clicked on node ID:%d\n", it->ID);*/
                        nodeid=it->ID;
                    }

                    /* ================= NODE CONTENT =====================*/
                    /*nk_layout_row_dynamic(ctx, 19, 7);*/
                    /*nk_layout_set_min_row_height(ctx, 30.);*/
                    /*nk_button_color(ctx, it->color);*/
                    enum {LR,SR};
                    int voltReg_radio;

                    char refdes[7]="U1"; char yeld[6]="0.9";   char R0[6]="93";
                    char Vi[6]="5.0";    char DV[6]="3.2";     char Vo[6]="1.8";
                    char Ii[6]="0.214";  char Iadj[6]="0.005"; char Io[6]="0.536";
                    char Pi[6]="1.072";  char Pd[6]="0.107";   char Po[6]="0.965";
                    //sprintf(it->values.refdes, "%d", it->ID);
                    //strncpy(it->values.refdes, refdes, 6);
                    strncpy(refdes, it->values.refdes, 6);
#if 0
                    // convert from double to string
                    snprintf(Vi, 6, "%g", it->values.Vi[0]);
                    snprintf(Ii, 6, "%g", it->values.Ii[0]);
                    snprintf(R0, 6, "%g", it->values.R[0]);
                    snprintf(Pi, 6, "%g", it->values.Pi[0]);
                    snprintf(yeld, 6, "%g", it->values.yeld);
                    snprintf(Iadj, 6, "%g", it->values.Iadj);
                    snprintf(DV, 6, "%g", it->values.DV);
                    snprintf(Pd, 6, "%g", it->values.Pd);
                    snprintf(Vo, 6, "%g", it->values.Vo);
                    snprintf(Io, 6, "%g", it->values.Io);
                    snprintf(Po, 6, "%g", it->values.Po);
#endif
                    static int LDin=1;
                    nTy* nd=&it->values; // so can use nd-> istead of it->values.
#define STEP 0.1
#define SPP  0.1

#if defined NODE_WIDTH && NODE_WIDTH == 256
#define EFW 50
#else
#define EFW 70 // and use NODE_WIDTH  316
#endif
                    if (!strncasecmp(it->name, "LD", 2)) { // Loads
                       const float size0[] = {35, 80, 45};
                       nk_layout_row(ctx, NK_STATIC, 20, 3, size0);
                       nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, refdes, 7, 0); nk_label(ctx, "Inputs:", NK_TEXT_RIGHT); nk_property_int(ctx, "###n", 1, &LDin, MaxIns, 1, 1);
//                       const float size[] = {50, 15, 30, 50, 15, 50, 10};
                       const float size[] = {EFW, 15, 30, EFW, 15, EFW, 10};
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_property_double(ctx, "###n", 0, &nd->Vi[0], DBL_MAX, STEP, SPP); nk_label(ctx, "V", NK_TEXT_LEFT); nk_label(ctx, "", NK_TEXT_RIGHT);      nk_label(ctx, "", NK_TEXT_RIGHT);                                nk_label(ctx, "", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->R[0], DBL_MAX, STEP, SPP);  nk_label(ctx, u8"ΩOhm", NK_TEXT_LEFT); /* Ω */
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_property_double(ctx, "###n", 0, &nd->Ii[0], DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT); nk_label(ctx, "Pdis:", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->Pd, DBL_MAX, STEP, SPP); nk_label(ctx, "W", NK_TEXT_LEFT); nk_property_double(ctx, "###n", 0, &nd->Pi[0], DBL_MAX, STEP, SPP); nk_label(ctx, "W", NK_TEXT_LEFT);
                    } else if (!strncasecmp(it->name, "IN", 2)) { // IN
                       const float size0[] = {35};
                       nk_layout_row(ctx, NK_STATIC, 20, 1, size0);
                       nk_label(ctx, "", NK_TEXT_RIGHT);
//                       const float size[] = {50, 15, 30, 50, 15, 50, 10};
                       const float size[] = {EFW, 15, 30, EFW, 15, EFW, 10};
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_label(ctx, "", NK_TEXT_RIGHT); nk_label(ctx, "", NK_TEXT_RIGHT); nk_label(ctx, "", NK_TEXT_RIGHT);      nk_label(ctx, "", NK_TEXT_RIGHT);                                nk_label(ctx, "", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->Vo, DBL_MAX, STEP, SPP); nk_label(ctx, "V", NK_TEXT_LEFT);
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_label(ctx, "", NK_TEXT_RIGHT); nk_label(ctx, "", NK_TEXT_RIGHT); nk_label(ctx, "", NK_TEXT_RIGHT);      nk_label(ctx, "", NK_TEXT_RIGHT);                                nk_label(ctx, "", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->Io, DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT);
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_label(ctx, "", NK_TEXT_RIGHT); nk_label(ctx, "", NK_TEXT_RIGHT); nk_label(ctx, "Pdis:", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->Pd, DBL_MAX, STEP, SPP); nk_label(ctx, "W", NK_TEXT_LEFT); nk_property_double(ctx, "###n", 0, &nd->Po, DBL_MAX, STEP, SPP); nk_label(ctx, "W", NK_TEXT_LEFT);
                    } else { // VoltReg
                       const float size0[] = {35, 55, 60, 80};
                       nk_layout_row(ctx, NK_STATIC, 20, 4, size0);
                       if (!strncasecmp(it->name, "LR", 2)) { // Linear
                          voltReg_radio=LR;
                       } else { // Switching
                          voltReg_radio=SR;
                       }
                       nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, refdes, 7, 0); nk_label(ctx, "VoltReg:", NK_TEXT_LEFT); if (nk_option_label(ctx, "Linear", voltReg_radio == LR)) voltReg_radio=LR; if (nk_option_label(ctx, "Switching", voltReg_radio == SR)) voltReg_radio=SR;
//                       const float size[] = {50, 15, 30, 50, 15, 50, 10};
                       const float size[] = {EFW, 15, 30, EFW, 15, EFW, 10};
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_property_double(ctx, "###n", 0, &nd->Vi[0], DBL_MAX, STEP, SPP); nk_label(ctx, "V", NK_TEXT_LEFT); nk_label(ctx, u8"ΔDV:", NK_TEXT_RIGHT);  nk_property_double(ctx, "###n", 0, &nd->DV, DBL_MAX, STEP, SPP);   nk_label(ctx, "V", NK_TEXT_LEFT); nk_property_double(ctx, "###n", 0, &nd->Vo, DBL_MAX, STEP, SPP); nk_label(ctx, "V", NK_TEXT_LEFT);
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       if (!strncasecmp(it->name, "LR", 2)) { // Linear
                       nk_property_double(ctx, "###n", 0, &nd->Ii[0], DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT); nk_label(ctx, "Iadj:", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->Iadj, DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT); nk_property_double(ctx, "###n", 0, &nd->Io, DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT);
                       } else { // Switching
                       nk_property_double(ctx, "###n", 0, &nd->Ii[0], DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT); nk_label(ctx, "Yeld:", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->yeld, DBL_MAX, STEP, SPP); nk_label(ctx, "", NK_TEXT_LEFT);  nk_property_double(ctx, "###n", 0, &nd->Io, DBL_MAX, STEP, SPP); nk_label(ctx, "A", NK_TEXT_LEFT);
                       }
                       nk_layout_row(ctx, NK_STATIC, 20, 7, size);
                       nk_property_double(ctx, "###n", 0, &nd->Pi[0], DBL_MAX, STEP, SPP); nk_label(ctx, "W", NK_TEXT_LEFT); nk_label(ctx, "Pdis:", NK_TEXT_RIGHT); nk_property_double(ctx, "###n", 0, &nd->Pd, DBL_MAX, STEP, SPP);   nk_label(ctx, "W", NK_TEXT_LEFT); nk_property_double(ctx, "###n", 0, &nd->Po, DBL_MAX, STEP, SPP); nk_label(ctx, "W", NK_TEXT_LEFT);
                       if (voltReg_radio == LR) strcpy(it->name, "LR");
                       if (voltReg_radio == SR) strcpy(it->name, "SR");
                    }

                    strncpy(it->values.refdes, refdes, 6);
#if 0
                    // convert back from string to double
                    char* endPtr;
                    it->values.Vi[0] = strtod(Vi, &endPtr);
                    it->values.Ii[0] = strtod(Ii, &endPtr);
                    it->values.R[0] = strtod(R0, &endPtr);
                    it->values.Pi[0] = strtod(Pi, &endPtr);
                    it->values.yeld = strtod(yeld, &endPtr);
                    it->values.Iadj = strtod(Iadj, &endPtr);
                    it->values.DV = strtod(DV, &endPtr);
                    it->values.Pd = strtod(Pd, &endPtr);
                    it->values.Vo = strtod(Vo, &endPtr);
                    it->values.Io = strtod(Io, &endPtr);
                    it->values.Po = strtod(Po, &endPtr);
#endif
#if 0
                    nk_text(ctx, text, 5, NK_TEXT_LEFT);
                    const char label[]="inputs";
                    /*nk_label(ctx, label, NK_TEXT_LEFT);*/
                    nk_value_uint(ctx, label, 3);

                    static int val=1;
                    nk_property_int(ctx, label, 1, &val, 5, 1, 1.);

                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, text, 9, nk_filter_float);
#endif
                    /*it->color.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, it->color.r, 255, 1,1);*/
                    /*it->color.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, it->color.g, 255, 1,1);*/
                    /*it->color.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, it->color.b, 255, 1,1);*/
                    /*it->color.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, it->color.a, 255, 1,1);*/
                    /* ====================================================*/
                    nk_group_end(ctx);
                }
                {
                    /* node connector and linking */
                    float space;
                    struct nk_rect bounds;
                    bounds = nk_layout_space_rect_to_local(ctx, node->bounds);
                    /*printf("bounds:%p\n", bounds);*/
                    bounds.x += nodedit->scrolling.x;
                    bounds.y += nodedit->scrolling.y;
                    it->bounds = bounds;

                    /* output connector */
                    space = node->bounds.h / (float)((it->output_count) + 1);
                    for (n = 0; n < it->output_count; ++n) {
                        struct nk_rect circle;
                        circle.x = node->bounds.x + node->bounds.w-4;
                        circle.y = node->bounds.y + space * (float)(n+1);
                        circle.w = 8; circle.h = 8;
                        nk_fill_circle(canvas, circle, nk_rgb(255, 255, 255));

                        /* start linking process */
                        if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true)) {
                            nodedit->linking.active = nk_true;
                            nodedit->linking.node = it;
                            nodedit->linking.input_id = it->ID;
                            nodedit->linking.input_slot = n;
                        }

                        /* draw curve from linked node slot to mouse position */
                        if (nodedit->linking.active && nodedit->linking.node == it &&
                            nodedit->linking.input_slot == n) {
                            struct nk_vec2 l0 = nk_vec2(circle.x + 3, circle.y + 3);
                            struct nk_vec2 l1 = in->mouse.pos;
                            nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
                                l1.x - 50.0f, l1.y, l1.x, l1.y, 1.0f, nk_rgb(255, 255, 255));
                        }
                    }

                    /* input connector */
                    space = node->bounds.h / (float)((it->input_count) + 1);
                    for (n = 0; n < it->input_count; ++n) {
                        struct nk_rect circle;
                        circle.x = node->bounds.x-4;
                        circle.y = node->bounds.y + space * (float)(n+1);
                        circle.w = 8; circle.h = 8;
                        nk_fill_circle(canvas, circle, nk_rgb(255, 255, 255));

                        /* end linking process */
                        int incon=0; /* check if this input is already connected */
                        struct node_link* linkPtr=nodeEditor.firstlink;
                        for (int l=0; l<nodeEditor.link_count; l++) {
                           if (linkPtr->output_id==it->ID &&
                               linkPtr->output_slot==n) incon=1;
                           linkPtr=linkPtr->next;
                        }
                        if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
                            nk_input_is_mouse_hovering_rect(in, circle) &&
                            nodedit->linking.active && nodedit->linking.node != it &&
                            incon==0 ) { /* avoid 2 outputs to 1 input */
                            nodedit->linking.active = nk_false;
                            node_editor_link(nodedit, nodedit->linking.input_id,
                                             nodedit->linking.input_slot, it->ID, n);
                        }

                        /* check if click on input */
                        struct nk_rect rect;
                        rect.x = circle.x+5;
                        rect.y = circle.y+5;
                        rect.w = 100; rect.h = 30;
                        if(nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, rect, nk_true)) {
                           inclick=1; /* check if click on output */
                           struct node_link* linkPtr=nodeEditor.firstlink;
                           for (int l=0; l<nodeEditor.link_count; l++) {
                              if (linkPtr->output_id==it->ID &&
                                  linkPtr->output_slot==n) {
                                 id=it->ID;
                                 inp=n;
                                 linkSavePtr=linkPtr;
                                 /*printf("clicked on link:%d\n", link);
                                 printf("node id:%d inp:%d\n", id, inp);*/
                              }
                              linkPtr=linkPtr->next;
                           }
                        }
                    }
                }
                it = it->next;
            } // while nodes

            /* reset linking connection */
            if (nodedit->linking.active && nk_input_is_mouse_released(in, NK_BUTTON_LEFT)) {
                nodedit->linking.active = nk_false;
                nodedit->linking.node = NULL;
                fprintf(stdout, "linking failed\n");
            }

            /* draw each link */
            struct node_link* linkPtr=nodeEditor.firstlink;
            for (int l=0; l<nodeEditor.link_count; l++) {
                struct node *ni = node_editor_find(nodedit, linkPtr->input_id);
                //printf("ni:%p id:%d\n", ni, linkPtr->input_id);
                struct node *no = node_editor_find(nodedit, linkPtr->output_id);
                //printf("no:%p id:%d\n", no, linkPtr->output_id);
                float spacei = node->bounds.h / (float)((ni->output_count) + 1);
                float spaceo = node->bounds.h / (float)((no->input_count) + 1);
                struct nk_vec2 l0 = nk_layout_space_to_screen(ctx,
                    nk_vec2(ni->bounds.x + ni->bounds.w, 3.0f + ni->bounds.y + spacei * (float)(linkPtr->input_slot+1)));
                struct nk_vec2 l1 = nk_layout_space_to_screen(ctx,
                    nk_vec2(no->bounds.x, 3.0f + no->bounds.y + spaceo * (float)(linkPtr->output_slot+1)));

                l0.x -= nodedit->scrolling.x;
                l0.y -= nodedit->scrolling.y;
                l1.x -= nodedit->scrolling.x;
                l1.y -= nodedit->scrolling.y;
                nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
                    l1.x - 50.0f, l1.y, l1.x, l1.y, 1.0f, nk_rgb(255, 255, 255));
                linkPtr=linkPtr->next;
            }

            if (updated) {
                /* reshuffle nodes to have least recently selected node on top */
                node_editor_pop(nodedit, updated);
                node_editor_push(nodedit, updated);
            }

            /* node selection */
            if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nk_layout_space_bounds(ctx))) {
                it = nodedit->begin;
                nodedit->selected = NULL;
                nodedit->bounds = nk_rect(in->mouse.pos.x, in->mouse.pos.y, 100, 200);
                while (it) {
                    struct nk_rect b = nk_layout_space_rect_to_screen(ctx, it->bounds);
                    b.x -= nodedit->scrolling.x;
                    b.y -= nodedit->scrolling.y;
                    if (nk_input_is_mouse_hovering_rect(in, b))
                        nodedit->selected = it;
                    it = it->next;
                }
            }

            /* contextual menu */
            if (nk_contextual_begin(ctx, 0, nk_vec2(100, 250), nk_window_get_bounds(ctx))) {
                const char *grid_option[] = {"Show Grid", "Hide Grid"};
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_contextual_item_label(ctx, "Del Link", NK_TEXT_CENTERED) &&
                    inclick==1) {
                    printf("delete link:%p\n", linkSavePtr);
                    printf("node id:%d inp:%d\n", id, inp);
                    node_editor_unlink(nodedit, linkSavePtr);
                    printf("\n");
                }
                if (nk_contextual_item_label(ctx, "Del Node", NK_TEXT_CENTERED)) {
                   if (nodeid>0) { // cannot remove node 0 IN
                      printf("delete node id:%d\n", nodeid);
                      node_editor_del(nodedit, nodeid);
                      printf("\n");
                      nodeclick=0;
                   }
                }
                if (nk_contextual_item_label(ctx, "New Reg", NK_TEXT_CENTERED))
                    node_editor_add(nodedit, "Reg", nk_rect(400, 260, NODE_WIDTH, NODE_HEIGHT),
                            nk_rgb(255, 255, 255), 1, 1);
                if (nk_contextual_item_label(ctx, "New Load", NK_TEXT_CENTERED))
                    node_editor_add(nodedit, "LDx", nk_rect(400, 260, NODE_WIDTH, NODE_HEIGHT),
                            nk_rgb(255, 255, 255), 1, 0);
                if (nk_contextual_item_label(ctx, "Load INI", NK_TEXT_CENTERED)) {
                    printf("load INI file\n");
                    loadINIres(nodedit);
                }
                if (nk_contextual_item_label(ctx, "Save INI", NK_TEXT_CENTERED)) {
                    printf("save INI file\n");
                    saveINI(nodedit);
                }
                if (nk_contextual_item_label(ctx, "Calc INI", NK_TEXT_CENTERED)) {
                    printf("save INI file\n");
                    saveINI(nodedit);
                    printf("calc INI file\n");
                    calcINI();
                }
                if (nk_contextual_item_label(ctx, grid_option[nodedit->show_grid],NK_TEXT_CENTERED))
                    nodedit->show_grid = !nodedit->show_grid;
                nk_contextual_end(ctx);
            }
        }
        nk_layout_space_end(ctx);

        /* window content scrolling */
        if (nk_input_is_mouse_hovering_rect(in, nk_window_get_bounds(ctx)) &&
            nk_input_is_mouse_down(in, NK_BUTTON_MIDDLE)) {
            nodedit->scrolling.x += in->mouse.delta.x;
            nodedit->scrolling.y += in->mouse.delta.y;
        }
    }
    nk_end(ctx);
    return !nk_window_is_closed(ctx, "NodeEdit");
}

