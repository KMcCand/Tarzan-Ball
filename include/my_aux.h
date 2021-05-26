#ifndef __AUX_H__
#define __AUX_H__


#include "body.h"

/**
 * A structure that holds the force constant,
 * and the bodies that the force pertains to.
 */
typedef struct aux aux_t;

/**
 * Allocates memory for an empty aux.
 * Automatically allocates space for 2 bodies by default.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new aux
 */
aux_t *aux_init(double constant, body_t *body1, body_t *body2);

/**
 * Gets the force constant of an aux.
 *
 * @param aux pointer to a aux returned from aux_init()
 * @return the aux's force constant
 */
double aux_get_constant(aux_t *aux);

/**
 * Returns true if the bodies in aux were colliding last frame, else returns false.
 * 
 * @param aux a pointer to an aux returned from aux_init()
 */ 
bool aux_get_collided_last_frame(aux_t *aux);

/**
 * Gets the first body of an aux.
 *
 * @param aux a pointer to a aux returned from aux_init()
 * @return the aux's first body
 */
body_t *aux_get_body1(aux_t *aux);

/**
 * Gets the second body of an aux.
 *
 * @param aux a pointer to a aux returned from aux_init()
 * @return the aux's second body
 */
body_t *aux_get_body2(aux_t *aux);

/**
 * Releases the memory allocated for an aux.
 *
 * @param aux a pointer to a aux returned from aux_init()
 */
void aux_free(aux_t *aux);

/**
 * Returns the collision of an aux_t
 *
 * @param aux a pointer to a aux returned from aux_init()
 */
void *aux_get_collision(aux_t *aux);

/**
 * Returns the info field for an aux_t
 *
 * @param aux a pointer to a aux returned from aux_init()
 */
void *aux_get_aux_info(aux_t *aux);

/**
 * Sets the collision for an aux_t
 *
 * @param aux a pointer to a aux returned from aux_init()
 * @param collision the new collision
 */
void aux_set_collision(aux_t *aux, void *collision);

/**
 * Sets the info field for an aux_t
 *
 * @param aux a pointer to a aux returned from aux_init()
 * @param info the information to be stored in the info field
 */
void aux_set_aux_info(aux_t *aux, void *info);

/**
 * Sets the colliding last frame status for the bodies in aux to new_val
 * 
 * @param aux a pointer to an aux returned from aux_init()
 * @param bool the new true or false value for if the bodies were colliding last frame
 */ 
void aux_set_collided_last_frame(aux_t *aux, bool new_val);


#endif
