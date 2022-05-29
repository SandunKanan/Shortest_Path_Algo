/* Program to find the fastest travel time to each intersection from a vehicle,
   where the vehicles are at certain intersections

   Written by Sandun Kanangama, October 2017.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DO_NOT_PASS 999
#define MAX_COST 999

#define INITIAL_LOCS 1

#define NUM_DIRECTIONS 4
#define EAST 0
#define NORTH 1
#define WEST 2
#define SOUTH 3


/**********************************************************************/
/* type definitions --------------------------------------------------*/

typedef struct {
	int x;
	int y;	
} dim_t;		/* Stores the dimensions of the grid */


typedef struct {
	int x;
	char y;
} point_t;		/* Stores the coordinates of a point on the grid */

typedef struct {
	point_t loc;
	int cost[NUM_DIRECTIONS];
} times_t;		/* Stores a point on the grid and the costs for travelling in
				 * each direction from that point */
typedef struct {
	dim_t dim;
	times_t *times;
	point_t *locs;
	int size;
	int num_locs;
} data_t;		/* Stores all the input data in a single struct */

typedef struct {
	point_t from;
	point_t loc;
	int min_cost;
	int prev_cost;
} path_t;		/* Stores a point and the minimum cost get getting to that 
				 * point from the first location specified*/

/**********************************************************************/
/* function prototypes -----------------------------------------------*/

data_t* store_input_data();
int get_dimensions(dim_t *coordinates);
times_t* get_travel_times(int size);
point_t* get_grid_locs(int *num_locs);
void do_stage1(data_t *data);
int count_unusable_paths(data_t *data);
int calculate_total_cost(data_t *data);
void do_stage2(data_t *data);
path_t* initialise_opt_path(data_t *data);
void find_all_optimal_paths(data_t *data, path_t *opt_path);
void start_point_zero(point_t pnt, path_t *paths);
void update_min_costs(data_t *data, path_t *opt_path);
int run_update_iteration(data_t *data, path_t *opt_path);
path_t* find_next_point(data_t *data, path_t *path, int curr_key, int dir);
void create_path_arrays(data_t *data, path_t *opt_path);
void print_s2_output(data_t *data, path_t *opt_path);
void free_data(data_t *data);

/**********************************************************************/
/* where it all happens ----------------------------------------------*/

int
main(int argc, char *argv[]) {
	data_t *data = store_input_data();
	do_stage1(data);
	do_stage2(data);
	free_data(data);
	data = NULL;
	return 0;
}

/**********************************************************************/

data_t*
store_input_data() {
	int size, num_locs;
	
	/* Create one pointer to each data type */
	dim_t dim; times_t *times; point_t *locs; data_t *data ;
	
	size = get_dimensions(&dim);
	times = get_travel_times(size);
	locs = get_grid_locs(&num_locs);
	data = (data_t*) malloc(sizeof(data_t));
	
	data->dim = dim;
	data->times = times;
	data->locs = locs;
	data->size = size;
	data->num_locs = num_locs;
	
	return data;
}

/**********************************************************************/

int
get_dimensions(dim_t *dim) {
	scanf("%d %d", &(dim->x), &(dim->y));
	return (dim->x)*(dim->y);
}

/**********************************************************************/

times_t*
get_travel_times(int size) {
	int i;
	times_t *times;
	times = (times_t*) malloc(size * sizeof(times_t));
	for (i=0;i<size;i++) {
		scanf("%d%c %d %d %d %d",
			&(times[i].loc.x),
			&(times[i].loc.y),
			&(times[i].cost[EAST]),
			&(times[i].cost[NORTH]),
			&(times[i].cost[WEST]),
			&(times[i].cost[SOUTH]));
	}
	return times;
}

/**********************************************************************/

point_t*
get_grid_locs(int *num_locs) {
	int i=0, current_size=INITIAL_LOCS;
	point_t *locs = (point_t*) malloc(INITIAL_LOCS*sizeof(point_t));
	while(scanf("%d%c\n", &(locs[i].x), &(locs[i].y)) == 2) {
		i++;
		if (i>=current_size) {
			current_size *= 2;
			locs = realloc(locs, current_size*sizeof(point_t));
		}
	}
	*num_locs = i;
	return locs;
}

/**********************************************************************/
/* Stage 1 -----------------------------------------------------------*/


void 
do_stage1(data_t *data) {
	
	printf("S1: grid is %d x %d, and has %d intersections\n", 
		data->dim.x, data->dim.y, data->size);	
	
	printf("S1: of %d possibilities, %d of them cannot be used\n",
		NUM_DIRECTIONS * data->size, 
		count_unusable_paths(data));
	
	printf("S1: total cost of remaining possibilities is %d seconds\n",
		calculate_total_cost(data));
	
	printf("S1: %d grid locs supplied, ", data->num_locs);
	
	printf("first one is %d%c, last one is %d%c\n",
		data->locs[0].x, data->locs[0].y,
		data->locs[data->num_locs-1].x, 
		data->locs[data->num_locs-1].y);
}

/**********************************************************************/
int
count_unusable_paths(data_t *data) {
	int i, j, num_paths=0;
	for (i=0; i<(data->size); i++) {
		for (j=0; j<NUM_DIRECTIONS; j++) {
			if (data->times[i].cost[j] == DO_NOT_PASS) {
				num_paths++;
			}
		}
	}
	return num_paths;
}

/**********************************************************************/

int
calculate_total_cost(data_t *data) {
	int i, j, cost=0;
	for (i=0; i<(data->size); i++) {
		for (j=0; j<NUM_DIRECTIONS; j++) {
			if (data->times[i].cost[j] != DO_NOT_PASS) {
				cost += data->times[i].cost[j];
			}
		}
	}
	return cost;
}

/**********************************************************************/
/* Stage 2 -----------------------------------------------------------*/

void
do_stage2(data_t *data) {
	path_t *opt_path = initialise_opt_path(data);
	find_all_optimal_paths(data, opt_path);
	create_path_arrays(data, opt_path);
	print_s2_output(data, opt_path);
	free(opt_path);
	opt_path = NULL;
}

/**********************************************************************/

path_t* 
initialise_opt_path(data_t *data) {
	int i;
	path_t *opt_path = (path_t*) malloc(data->size * sizeof(path_t));	
	for (i=0; i<(data->size); i++) {
		opt_path[i].loc.x = data->times[i].loc.x;
		opt_path[i].loc.y = data->times[i].loc.y;
		opt_path[i].min_cost = MAX_COST;
		opt_path[i].prev_cost = MAX_COST;
		opt_path[i].from = opt_path[i].loc;
	}
	return opt_path;
}

/**********************************************************************/

void 
find_all_optimal_paths(data_t *data, path_t *opt_path) {
	start_point_zero(data->locs[0], opt_path);
	update_min_costs(data, opt_path);

}

/**********************************************************************/

void
start_point_zero(point_t pnt, path_t *opt_path) {
	int i=0;
	while ( !((pnt.x == opt_path[i].loc.x) && (pnt.y == opt_path[i].loc.y))) {
		i++;
	}
	opt_path[i].min_cost = 0;
}

/**********************************************************************/

void update_min_costs(data_t *data, path_t *opt_path) {
	int update_occurred=1;
	while (update_occurred) {
		update_occurred = run_update_iteration(data, opt_path);	
	}
}

/**********************************************************************/

int
run_update_iteration(data_t *data, path_t *opt_path) {
	int i, dir, update_occurred=0, road_cost;
	path_t curr_pt, *next_pt;
	
	for (i=0; i<(data->size); i++) {
		
		if ((opt_path[i].min_cost) < (opt_path[i].prev_cost)) {
			
			opt_path[i].prev_cost = opt_path[i].min_cost;
			for (dir=0; dir<NUM_DIRECTIONS; dir++) {
				
				curr_pt = opt_path[i];
				next_pt = find_next_point(data, opt_path, i, dir);
				road_cost = data->times[i].cost[dir];
				if ((road_cost != DO_NOT_PASS) && 
				(curr_pt.min_cost + road_cost < next_pt->min_cost)) {
				
					update_occurred = 1;
					next_pt->min_cost = curr_pt.min_cost + road_cost;
					next_pt->from = curr_pt.loc;
					
				} else if ((road_cost != DO_NOT_PASS) 			    && 
				(curr_pt.min_cost + road_cost == next_pt->min_cost) &&
				((curr_pt.loc.x < next_pt->from.x)||
				 ((curr_pt.loc.x == next_pt->from.x) && 
				 	 (curr_pt.loc.y < next_pt->from.y))))	{
				
					next_pt->from = curr_pt.loc;
				}
			}
		}
	}
	return update_occurred;
}

/**********************************************************************/

path_t*
find_next_point(data_t *data, path_t *path, int curr_key, int dir) {
	int array_shift;
	if (dir == EAST) {
		array_shift = 1;
	} else if (dir == NORTH) {
		array_shift = -(data->dim.x);
	} else if (dir == WEST) {
		array_shift = -1;
	} else {
		array_shift = data->dim.x;
	}
	return &(path[curr_key + array_shift]);
}

/**********************************************************************/

void 
create_path_arrays(data_t *data, path_t *opt_path) {
	/*int i;
	path_t **paths;*/
}

/**********************************************************************/

void 
print_s2_output(data_t *data, path_t *opt_path) {
	int i;
	for (i=1; i<(data->num_locs)-1; i++) {
		
	}
}
	
/**********************************************************************/

void 
free_data(data_t *data) {
	free(data->times);
	free(data->locs);
	free(data);
}

/** Algorithms are fun **/
