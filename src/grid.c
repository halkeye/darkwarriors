/* Grid Class */
#include "mud.h"
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef NULL
#define NULL
#endif

#ifndef CREATE
#define CREATE_FREE
#define CREATE(result, type, number)					\
do											\
{											\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {											\
		perror("malloc failure");						\
		fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
		abort();									\
    }											\
} while(0)
#endif

#ifndef DISPOSE
#define DISPOSE_FREE
#define DISPOSE(point)                         \
do                                             \
{                                              \
   if( (point) )                               \
   {                                           \
       free( (point) );                        \
      (point) = NULL;                          \
   }                                           \
} while(0)
#endif


/**
 * Creates a new GRID_DATA of length, width and height 
 */
GRID_DATA * grid_create(int base, int cols, int rows, int height)
{
	GRID_DATA * grid;
	CREATE(grid, GRID_DATA, 1);
	grid->base = base;
	grid->height = height-base;
	if (grid->height < 0) {
		bug("Invalid height - base");
		return NULL;
	}
	grid->width = cols-base;
	if (grid->width < 0) {
		bug("Invalid width - base");
		return NULL;
	}
	grid->length = rows-base;
	if (grid->length < 0) {
		bug("Invalid length - base");
		return NULL;
	}
	CREATE(grid->data, unsigned long, grid->width*grid->length*grid->height);
	return grid;
}

void grid_destroy(GRID_DATA * grid)
{
	DISPOSE(grid->data);
	DISPOSE(grid);
}

int grid_pos(GRID_DATA * grid, int col, int row, int height)
{
	return col+(row*grid->width)+(height*(grid->width*grid->length));
}

/**
 * Updates the coords based on the base value
 */
void grid_update_coords(GRID_DATA * grid, int * col, int * row, int * height)
{
	*col -= grid->base;
	*row -= grid->base;
	*height -= grid->base;
}

bool grid____valid_coors(GRID_DATA * grid, int col, int row, int height)
{
	/* Make sure this includes the upper limit */
	if (col < 0 || col >= grid->width) return FALSE;
	if (row < 0 || row >= grid->length) return FALSE;
	if (height < 0 || height >= grid->height) return FALSE;
	return TRUE;
}


/**
 * True if its valid coords, false if not
 */
bool grid_valid_coors(GRID_DATA * grid, int col, int row, int height)
{
	grid_update_coords(grid,&col,&row,&height);
    return grid____valid_coors(grid, col, row, height);
}

/**
 * Set the data at pos x,y,z
 */
void   grid_set_pos (GRID_DATA * grid, int col, int row, int height, void*item)
{
	grid_update_coords(grid,&col,&row,&height);
	if (!grid____valid_coors(grid, col, row, height)) {
		bug("Non valid coord passed in");
		return;
	}
    int pos = grid_pos(grid, col, row, height);
	grid->data[pos] = (unsigned long) item;
	//grid->data[pos] = item;
}

/**
 * Get the data at pos x,y,z
 */
void * grid_get_pos (GRID_DATA * grid, int col, int row, int height)
{
	void * ret;
	grid_update_coords(grid,&col,&row,&height);
	if (!grid____valid_coors(grid, col, row, height)) {
		bug("Non valid coord passed in");
		return NULL;
	}
    int pos = grid_pos(grid, col, row, height);
	ret = (void *) grid->data[pos];
	return ret;
}
   

/** 
 * Find object in the grid, return the position in raw format
 * call grid_translate to get x,y,z pos
 */
int    grid_find (GRID_DATA * grid, void * obj, int * col, int * row, int * height)
{
	int a,b,c;
	for ( a = 0; a < grid->height; a++) {
		for ( c = 0; c < grid->width; c++) {
			for ( b = 0; b < grid->length; b++) {
				int pos = grid_pos(grid, a, b, c);
				bug("pos = %d, %d, %d, %d, %ld", pos, a,b,c,grid->data[pos] );
				if (grid->data[pos] == (unsigned long) obj) {
					*col = c;
					*row = b;
					*height = a;
					return pos;
				}
			}
		}
	}
	return -1;
}

/**
 * Translate grid position to coordinates
 */
void   grid_translate (GRID_DATA * grid, int origpos, int * cols, int * rows, int * height)
{
	*height = 0;
	*cols = 0;
	*rows = 0;
	while (origpos > (grid->width*grid->length)) {
		origpos -= (grid->width*grid->length);
		*height += 1;
	}
	while (origpos > grid->length) {
		origpos -= grid->length;
		*rows += 1;
	}
	*cols = origpos;
	/* This should be right */
}

#ifdef DISPOSE_FREE
#undef DISPOSE
#endif
#ifdef CREATE_FREE
#undef CREATE
#endif
