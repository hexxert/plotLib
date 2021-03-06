#include "interface.h"

/* 
 * This file makes the interface between the plotLib thread 
 * and the aplication using it...
 *
 */


/* declare the global lib state here */
plotLib_t pL;

/* function responsible to keep the object updated on screen */
#ifdef WIN32
void plotlib_thread(void * arg)
#elif defined __linux__ || defined __FreeBSD__ || defined __APPLE__
void * plotlib_thread(void * arg)
#endif
{
	int fake = 0;
	glutInit(&fake, NULL);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	/* register drawing functions */
	glutDisplayFunc(plt_draw);
	glutIdleFunc(plt_idle);
	
	while(1){
		/* query plotLib thread msg (should this thread exit?)*/
		if(pL.thread_msg == thread_msg_exit)
			break;
			
		/* verify that the window for where 
		 * every plot must be drawn is open
		 * if not create the window...
		 */
		check_plts_windows();
	
		/* call glut to render and handle events */
		glutMainLoopUpdate();
	}
	
	/* shutdown glut */
	
}

/* show current plots */
/*
 * function needs variable number of arguments
 *
 */
void plt_show(const plot_t * plt){

	/* register thread in the queue */
	C_SAFE_CALL( register_plt(plt) );
	
	/* thread may be in the queue,
	 * but will not be updated 
	 * unless we mark it to redraw
	 */
	C_SAFE_CALL( queue_plt_redraw(plt) );
	
	SET_API_ERROR(API_SUCCESS);
	return;
error:

	SET_API_ERROR();
}

/* add another layer to the current plot
 *
 */
 #define plt_grab_minmax(var, ind, min_v, max_v)	\
		 do{														\
			if(var!=NULL){										\
				if(!(isnan(var[ind])==true ||				\
					isinf(var[ind]) == true) ){				\
					if(var[ind]>max_v){						\
						max_v = var[ind];					\
					}												\
					if(var[ind]<min_v){						\
						min_v = var[ind];						\
					}												\
				}else{											\
					C_LOG_MSG("invalid number");		\
				}													\
			}											\
		}while(0)			

#define plt_ss_vals(var, min_v, max_v)				\
		do{														\
			if(var!=NULL){										\
				min_v = var[0];								\
				max_v = var[0];								\
			}														\
		}while(0)
		
void plt_add_layer(plot_t * plt, const double * x, const double * y, const double * z, size_t size){
	plt_layer_t * layer = NULL;
	int32_t i = 0;
	
	C_CHECK_CONDITION( plt == NULL, API_BAD_INPUT);
	C_CHECK_CONDITION( size <= 0, API_BAD_INPUT);
	C_CHECK_CONDITION( plt->plt_type <= 0, API_BAD_INPUT);
	C_CHECK_CONDITION( plt->plt_type >= PL_NUM_PLOT_TYPES, API_BAD_INPUT); 
	/* 
	 * validate plot requirements  
	 * 
	 */
	C_CHECK_CONDITION( plt_val_req_func_ptr[plt->plt_type]==NULL, API_BAD_INPUT);
	C_CHECK_CONDITION( plt_aux_data_func_ptr[plt->plt_type]==NULL, API_BAD_INPUT);
	C_SAFE_CALL( plt_val_req_func_ptr[plt->plt_type](plt, x, y, z ) );
	
	/* allocate memory for the layer */
	C_SAFE_CALL(layer = mem_alloc( sizeof(plt_layer_t), true) );
	
	/* allocate memory for data */
	C_SAFE_CALL(layer->x = mem_alloc(3 * size * sizeof(double), true) ); 
	layer->y = &layer->x[size];
	layer->z = &layer->y[size];
	
	/* copy data */
	if(x != NULL){ memcpy(layer->x, x, size * sizeof(double)); }
	if(y != NULL){ memcpy(layer->y, y, size * sizeof(double)); }
	if(z != NULL){ memcpy(layer->z, z, size * sizeof(double)); }
	
	/* adjust mapping parameters */
	
	/* if parameters have never been set 
	 * we need to start with the values 
	 * of the array
	 */
	if(plt->map.state == false){
		plt_ss_vals(x, plt->map.xmin, plt->map.xmax);
		plt_ss_vals(y, plt->map.ymin, plt->map.ymax);
		plt_ss_vals(z, plt->map.zmin, plt->map.zmax);		
		plt->map.state = true;
	}
	
	for(i = 0; i < size; i++){
		/* validate data: nans and infs don't count */
		plt_grab_minmax(x, i, plt->map.xmin, plt->map.xmax);
		plt_grab_minmax(y, i, plt->map.ymin, plt->map.ymax);
		plt_grab_minmax(z, i, plt->map.zmin, plt->map.zmax);
	}
	
	/* optional parameters such as color 
	 * and linestyle are to be given later
	 * for now just set them to none
	 */
	C_SAFE_CALL( layer->aux_layer_data = plt_aux_data_func_ptr[plt->plt_type](plt->num_layers) );
	
	/* add layer to plot and increase num layers */
	C_SAFE_CALL( mem_realloc(plt->layers, (plt->num_layers++) ) );
	plt->layers[plt->num_layers-1] = layer;
	
	SET_API_ERROR( API_SUCCESS );
	return;
error:
	/* free layer */
	free_mem(layer);
	SET_API_ERROR( API_PLT_ADD_LAYER );
}



/* 
 * multiplot 
 * fixed inputs: plot dimension
 * variable inputs : plots 
 *  -> order of plots matters 
 * 		-> plots are displayed left to right and top to down
 * 		-> to ensure that the format is correct this function should be
 *		   called in the following manner multiplot(xdim,ydim, ..., NULL);
 */
void plt_multiplot(int32_t dim_x, int32_t dim_y, ...){
	static int32_t link = 1;
	va_list ap, ap_c;
	plot_t * plt;
	int32_t i = 0;
	
	C_CHECK_CONDITION( dim_x * dim_y <= 0, API_MATRIX_BAD_FORM );
	
	va_start(ap,dim_y); 
	va_copy(ap_c, ap);
	
	for(i = 0; i < dim_x * dim_y; i++){
		/* consume arguments one by one */
		plt = va_arg(ap, plot_t *);
		
		C_CHECK_CONDITION( plt == NULL && (i+1) <= dim_x*dim_y, API_BAD_MATRIX_PLT);
	}
	
	/* check if the last argument is null if it is not give error*/
	plt = va_arg(ap, plot_t *);
	C_CHECK_CONDITION( plt!=NULL, API_BAD_MATRIX_INPUT);
	
	va_end(ap);
	
	/* now that inputs have been verified input, lets work */		
	for(i = 0; i < dim_x * dim_y ; i++){
		plt = va_arg(ap_c, plot_t *);
		plt->subplot_state = true;
		plt->subplot_dim[0] = dim_x;
		plt->subplot_dim[1] = dim_y;
		plt->subplot_num = i;
		plt->subplot_link = link;
	}
	va_end(ap_c);
	
	/* increment link */
	link++;
	
error:
	return;
}

/* 
 * Before calling the drawn routines check if there is any window we need to open...
 *
 */
#define VAR_NAME_LIMIT 128
void check_plts_windows(void){
	int32_t i = 0, j = 0;
	int32_t link = 0;
	
	char buffer[VAR_NAME_LIMIT];
	for(i = 0; i < pL.num_plts; i++){
		if( pL.window_handle[i] == -1 ){
			/* is subplotting enabled ? */
			if( pL.plts[i]->subplot_state == true ){
				/* locate all plots 
				 * belonging to this subplot
				 * and open a common window 
				 * ! multiplot should destroy 
				 *   object window if it has one
				 */
				 link = pL.plts[i]->subplot_link;
				 snprintf(buffer,VAR_NAME_LIMIT,"subplot_%d",link);
				 
				 /* use variable name or subplot + link name */
				 pL.window_handle[i] = glutCreateWindow(buffer);
				 glutInitWindowSize(pL.window_w[i],pL.window_h[j]);
				 
				 /* set reshape callback */
				 glutReshapeFunc(plt_reshape_window);
				 
				 /* get the plots belonging to this subplot */
				 for(j = i+1; j < pL.num_plts; j++){
					if( pL.plts[j]->subplot_link == link){
						/* 
						 * if they already have a window and 
						 * is the same as this one don't
						 * do nothing otherwise kill it
						 */
						if( pL.window_handle[j] != -1){
							if(pL.window_handle[j] != pL.window_handle[i]){
								glutDestroyWindow(pL.window_handle[j]);
							}
							pL.window_handle[j] = pL.window_handle[i];
						}
					}
				 }
				 
			}else{
			     snprintf(buffer,VAR_NAME_LIMIT,"plot_%d",i);
				 /* use variable name or subplot + link name */
				 pL.window_handle[i] = glutCreateWindow(buffer);
				 glutInitWindowSize(pL.window_w[i],pL.window_h[j]);
				 glutReshapeFunc(plt_reshape_window);
			}
		}
	}
}

/* each time a show command is issued 
 * we need to query if the object is in the queue 
 */
 void register_plt(const plot_t * plt){
	int32_t i = 0;
	bool lock_acquired = false;
	
	C_CHECK_CONDITION( plt == NULL, API_BAD_INPUT);
	
	/* verify if plt is in the queue */
	for(i = 0; i < pL.num_plts; i++){
		if(pL.plts[i]==plt)
			break;
	}
	
	/* add plot to queue */
	if(i >= pL.num_plts){
		C_SAFE_CALL( acquire_lock() );
		lock_acquired = true;
		
		/* allocate new plt in queue */
		C_SAFE_CALL( pL.plts = mem_realloc(
							pL.plts, (pL.num_plts+1) * sizeof(plot_t *)) 
					);
					
		C_SAFE_CALL( pL.window_handle = mem_realloc(
							pL.window_handle, (pL.num_plts+1) * sizeof(int32_t))
					);
		C_SAFE_CALL( pL.window_w = mem_realloc(
							pL.window_w, (pL.num_plts+1) * sizeof(int32_t))
					);
		C_SAFE_CALL( pL.window_h = mem_realloc(
							pL.window_h, (pL.num_plts+1) * sizeof(int32_t))
					);
			
		pL.num_plts++;
		
		/* add new plt to the queue */
		pL.plts[pL.num_plts-1] = (plot_t *)plt;
		
		/* needs to be sorted out!!! 
		 * the variables need to be filled before registering 
		 * (maybe we need to have them on each plot and copy them to pL)
		 */
		
		pL.window_handle[pL.num_plts-1] = -1;
		pL.window_w[pL.num_plts-1] = 400;
		pL.window_h[pL.num_plts-1] = 350;
		C_SAFE_CALL( release_lock() );
		lock_acquired = false;
	}
	
	
	SET_API_ERROR(API_SUCCESS);
	return;
error:
	if(lock_acquired == true)
		release_lock();
	
	SET_API_ERROR(API_REGISTER_PLOT);
 }
 
 /* schedule plot to be drawn on screen */
 void queue_plt_redraw(const plot_t * plt){
	int32_t i = 0;
	bool lock_acquired = false;
	plt_layer_t * layer = NULL;
	/* verify if plt is in the queue */
	for(i = 0; i < pL.num_plts; i++){
		if(pL.plts[i]==plt)
			break;
	}
	if( i >= pL.num_plts){
		printf("Warning plot is unregistered! \t registering it for you...\n");
		C_SAFE_CALL( register_plt( plt ) );
		
		for(i = 0; i < pL.num_plts; i++){
			if(pL.plts[i]==plt)
				break;
		}
	}
	
	C_SAFE_CALL( acquire_lock() );
	lock_acquired = true;
	
	/* for each layer in plot commit data to screen */
	for(i=0; i<plt->num_layers; i++){
		layer = plt->layers[i];
		/* verify if the size is the same if it is not reallocate */
		if(layer->size != layer->size_data){
			C_SAFE_CALL( layer->xdata = mem_realloc(
							layer->xdata, layer->size * sizeof(plot_t *)) 
					);
			C_SAFE_CALL( layer->ydata = mem_realloc(
							layer->ydata, layer->size * sizeof(plot_t *)) 
					);
			C_SAFE_CALL( layer->zdata = mem_realloc(
							layer->zdata, layer->size * sizeof(plot_t *)) 
					);
			layer->size_data = layer->size;
		}
		/* copy data */
		memcpy(layer->xdata, layer->x, layer->size * sizeof(double));
		memcpy(layer->ydata, layer->y, layer->size * sizeof(double));
		memcpy(layer->zdata, layer->z, layer->size * sizeof(double));
	}
	
	C_SAFE_CALL( release_lock() );
	lock_acquired = false;
	
	SET_API_ERROR( API_SUCCESS );
	return;
error:
	if(lock_acquired == true)
		release_lock();
		
	SET_API_ERROR( API_QUEUE_PLOT );
 }
 
 /* unregister plot from queue */
 void unregister_plt(plot_t * plt){
	int32_t i = 0, j = 0;
	int32_t link = 0, window_handle = 0;
	bool lock_state = false;
	
	C_CHECK_CONDITION( plt == NULL, API_BAD_INPUT);
	
	C_SAFE_CALL( acquire_lock() );
	lock_state = true;
	
	/* verify if plt is in the queue */
	for(i = 0; i < pL.num_plts; i++){
		if(pL.plts[i]==plt)
			break;
	}
	if(i >= pL.num_plts){
		return;
	}
	
	window_handle = pL.window_handle[i];
	
	/* remove plt and other things from queue */
	for(j=i+1; j < pL.num_plts;j++){
		pL.plts[j-1] = pL.plts[j];
		pL.window_handle[j-1] = pL.window_handle[j];
		pL.window_w[j-1] = pL.window_w[j];
		pL.window_h[j-1] = pL.window_h[j];
	}
	
	pL.num_plts--;
	
	/* reescale input */
	
	/* destroy plt window */
	/* in subplotting do it only if the graphs 
	 * belonging to the same window have all been 
	 * unregistered 
	 */
	
	if(plt->subplot_state){
		
		/* get subplot link value */
		C_CHECK_CONDITION( (link = plt -> subplot_link) == 0, API_INVALID_SUBPLOT_LINK);
		
		/* look for plots with the same link */
		for(j=0; j < pL.num_plts; j++ )
			if((pL.plts[j])->subplot_link == link)
				break;
	
		/* not found -> destroy plt window */
		if(j>=pL.num_plts && window_handle > 0)
			/* kill window */
			glutDestroyWindow(window_handle);
	}else{
		/* just kill the window */
		if(window_handle > 0)
			glutDestroyWindow(window_handle);
	}

	C_SAFE_CALL( release_lock() );
 
	SET_API_ERROR(API_SUCCESS);
	return;
 error:
	if(lock_state)
		release_lock();
		
	SET_API_ERROR(API_UNREG_PLOT);
 }
 
 /* return only when the lock is acquired */
 void acquire_lock(void){
	#ifdef WIN32
		int32_t ct = 0;
		while( WaitForSingleObject( pL.lock, INFINITE) != WAIT_OBJECT_0 || ct < 10)
			ct++;
	#elif defined __linux__ || defined __FreeBSD__ || defined __APPLE__
		 pthread_mutex_lock( &pL.lock );
	#endif
 }
 
 /* release lock */
 void release_lock(void){
	#ifdef WIN32
		ReleaseMutex(pL.lock);
	#elif defined __linux__ || defined __FreeBSD__ || defined __APPLE__
		pthread_mutex_unlock( &pL.lock );
	#endif
 }
