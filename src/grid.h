#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct grid_data {
   unsigned long * data;
   int length;
   int width;
   int height;
   int base; /* 0 base or above */
};
typedef struct grid_data GRID_DATA;

/**
 * Creates a new GRID_DATA of length, width and height 
 */
GRID_DATA * grid_create(int base, int cols, int rows, int height);
void grid_destroy(GRID_DATA * grid);

/**
 * Updates the coords based on the base value
 */
void grid_update_coords(GRID_DATA * grid, int * col, int * row, int * height);

/**
 * True if its valid coords, false if not
 */
bool grid_valid_coors(GRID_DATA * grid, int col, int row, int height);

/**
 * Set the data at pos x,y,z
 */
void   grid_set_pos (GRID_DATA * grid, int col, int row, int height, void * item);

/**
 * Get the data at pos x,y,z
 */
void * grid_get_pos (GRID_DATA * grid, int col, int row, int height);

/** 
 * Find object in the grid, return the position in raw format
 * call grid_translate to get x,y,z pos
 */
//int    grid_find (GRID_DATA * grid, void * obj);
int    grid_find (GRID_DATA * grid, void * obj, int * col, int * row, int * height);

/**
 * Translate grid position to coordinates
 */
void   grid_translate (GRID_DATA * grid, int origpos, int * x, int * y, int * z);


#ifdef __cplusplus
class GRID {
   protected:
      GRID_DATA * grid;
   public:
      GRID(int base, int length, int width, int height) { 
         this->grid = grid_create(base,length,width,height); 
      }
      ~GRID() { grid_destroy(this->grid); this->grid = NULL; }

      inline void set(int col, int row, int height, void*item) { grid_set_pos(grid,col,row,height,item); }
      inline void * get(int col, int row, int height) { return grid_get_pos(grid,col,row,height); }
	  inline int find(void * obj, int * col, int * row, int * height) { return grid_find(grid,obj,col,row,height); }
	  inline bool valid(int x, int y, int z) { return grid_valid_coors(grid,x,y,z); }
	  inline void translate(int pos, int * x, int * y, int * z) { grid_translate(grid,pos, x,y,z); }

	  inline int width(void) { return grid->width; }
	  inline int length(void) { return grid->length; }
	  inline int height(void) { return grid->height; }
	  inline int base(void) { return grid->base; }
	  
	  inline void width(int width) { grid->width = width; }
	  inline void length(int length) { grid->length = length; }
	  inline void height(int height) { grid->height = height; }
	  inline void base(int base) { grid->base = base; }
};
#endif
